#ifndef EZVEC_H
#define EZVEC_H

#include "ezoption.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <functional>

#define EZVEC_DEV 1
#if EZVEC_DEV
#define EZVEC_IMPLEMENTATION
#endif // EZVEC_DEV

/* --------------- TODO ---------------
 * Slice find/rfind/contains predicate
 * Push &Vec/Slice/Copy
 * Comments from string library
 */

namespace ezvec {
template <typename T> class Slice;
template <typename T> class Vec;

template <typename T> class Slice {
  const T *ptr_;
  size_t len_;

public:
  Slice() : ptr_(nullptr) {}
  Slice(const T *ptr, size_t len) : ptr_(ptr), len_(len) {}
  Slice(const Slice<T> &other) : ptr_(other.ptr_), len_(other.len_) {}
  [[nodiscard]] const T *ptr() const { return ptr_; }
  [[nodiscard]] size_t len() const { return len_; }

  [[nodiscard]] Vec<T> to_owned() const;

  Slice<T> &operator=(const Slice<T> &other) {
    ptr_ = other.ptr_;
    len_ = other.len_;
    return *this;
  }

  [[nodiscard]] bool operator==(const Vec<T> &other) const;

  [[nodiscard]] bool operator==(const Slice<T> other) const {
    if (len() != other.len())
      return false;
    for (size_t i = 0; i < len(); i++)
      if (ptr()[i] != other[i])
        return false;
    return true;
  }

  [[nodiscard]] ezopt::Option<const T *> get(size_t i) const {
    return i < len() ? ezopt::Option(&ptr()[i]) : ezopt::Option<const T *>();
  }
  [[nodiscard]] const T &operator[](size_t i) const { return ptr()[i]; }
  [[nodiscard]] Slice<T> operator()(size_t i, size_t j) const {
    return Slice<T>(ptr() + i, j - i);
  }
  [[nodiscard]] ezopt::Option<Slice<T>> get(size_t i, size_t j) const {
    return j <= len() && i <= j ? ezopt::Option((*this)(i, j))
                                : ezopt::Option<Slice<T>>();
  }

  void unsafe_set_ptr(const T *ptr) { ptr_ = ptr; }
  void unsafe_set_len(size_t len) { len_ = len; }
  [[nodiscard]] T *unsafe_ptr_mut() const { return const_cast<T *>(ptr()); }
  [[nodiscard]] const T *begin() const { return ptr(); }
  [[nodiscard]] const T *end() const { return ptr() + len(); }
  [[nodiscard]] const T *rbegin() const { return end(); }
  [[nodiscard]] const T *rend() const { return begin(); }
  [[nodiscard]] bool is_empty() const { return len() == 0; }
  [[nodiscard]] bool is_valid() const { return ptr(); }
  [[nodiscard]] explicit operator bool() const {
    return !is_empty() && is_valid();
  }

  [[nodiscard]] ezopt::Option<size_t> find(const T &val) const {
    for (size_t i = 0; i < len(); i++)
      if (ptr()[i] == val)
        return i;
    return ezopt::Option<size_t>();
  }
  [[nodiscard]] ezopt::Option<size_t> rfind(const T &val) const {
    for (size_t i = len() - 1; i >= 0; i--)
      if (ptr()[i] == val)
        return i;
    return ezopt::Option<size_t>();
  }
  [[nodiscard]] bool contains(const T &val) const {
    return bool(this->find(val));
  }
};

template <typename T> class Vec {
  static constexpr size_t ELEMENT_SIZE = sizeof(T);
  static constexpr size_t INIT_CAP = 24;
  static constexpr size_t GROWTH_RATE = 2;
  Slice<T> sl_;
  size_t cap_;

public:
  const Slice<T> *operator->() const { return &sl_; }
  const Slice<T> &operator*() const { return sl_; }
  [[nodiscard]] size_t cap() const { return cap_; }
  void unsafe_set_cap(size_t cap) { this->cap_ = cap; }
  void unsafe_set_ptr(const T *ptr) { this->sl_.unsafe_set_ptr(ptr); }
  void unsafe_set_len(size_t len) { this->sl_.unsafe_set_len(len); }
  void unsafe_set_sl(Slice<T> sl) { this->sl_ = sl; }
  [[nodiscard]] T *ptr_mut() const { return const_cast<T *>(sl_.ptr()); }
  [[nodiscard]] Slice<T> as_slice() const { return sl_; }

  Vec(size_t cap) : cap_(std::max(cap, INIT_CAP)) {
    T *ptr = (T *)malloc(ELEMENT_SIZE * cap_);
    sl_ = {ptr, 0};
  }
  Vec() : Vec(INIT_CAP) {}
  Vec(Slice<T> sl) : cap_(std::max(sl.len(), INIT_CAP)) {
    T *ptr = (T *)malloc(ELEMENT_SIZE * cap());
    for (size_t i = 0; i < sl.len(); i++)
      ptr[i] = sl[i];
    sl_ = {ptr, sl.len()};
  }
  Vec(Vec<T> &&vec) : sl_(vec.sl_), cap_(vec.cap()) {
    vec.unsafe_set_ptr(nullptr);
  }
  Vec(const T *ptr, size_t len, size_t cap) : sl_({ptr, len}), cap_(cap) {}
  ~Vec() {
    for (size_t i = 0; i < sl_.len(); i++)
      ptr_mut()[i].~T();
    free(ptr_mut());
  }

  Vec<T> &operator=(Vec<T> &&other) {
    assert(ptr_mut() != other.ptr_mut());
    free(ptr_mut());
    sl_ = other.sl_;
    cap_ = other.cap();
    other.unsafe_set_ptr(nullptr);
    return *this;
  }
  Vec<T> &operator=(const Slice<T> &other) {
    if (other.len() > cap()) {
      free(ptr_mut());
      cap_ = other.len();
      unsafe_set_ptr((T *)malloc(ELEMENT_SIZE * cap()));
    }
    unsafe_set_len(other.len());
    for (size_t i = 0; i < sl_.len(); i++)
      ptr_mut()[i] = other[i];
    return *this;
  }

  void push(T &&val) {
    if (sl_.len() == cap()) {
      cap_ *= GROWTH_RATE;
      unsafe_set_ptr((T *)realloc(ptr_mut(), ELEMENT_SIZE * cap()));
    }
    ptr_mut()[sl_.len()] = val;
    unsafe_set_len(sl_.len() + 1);
  }

  [[nodiscard]] T &operator[](size_t i) const {
    return const_cast<T &>(sl_[i]);
  }
  [[nodiscard]] Slice<T> operator()(size_t i, size_t j) const {
    return sl_(i, j);
  }
  [[nodiscard]] bool operator==(const Slice<T> &other) const {
    return as_slice() == other;
  }
  [[nodiscard]] bool operator==(const Vec<T> &other) const {
    return as_slice() == other.as_slice();
  }
  [[nodiscard]] Vec<T> clone() const { return Slice<T>(as_slice()); }
  [[nodiscard]] T *move_ptr() {
    T *ptr = ptr_mut();
    unsafe_set_ptr(nullptr);
    return ptr;
  }
};

template <typename T> bool Slice<T>::operator==(const Vec<T> &other) const {
  return *this == other.as_slice();
}
template <typename T> Vec<T> Slice<T>::to_owned() const {
  return Vec<T>(*this);
}

} // namespace ezvec

#endif // !EZVEC_H
