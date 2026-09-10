#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <functional>
#include <utility>

#define EZSTR_DEV 1
#if EZSTR_DEV
#define EZSTR_IMPLEMENTATION
#endif

#ifndef EZSTR_H
#define EZSTR_H
namespace ezstr {

/* --------------- TODO --------------- */
/* Split
 * Replace
 */

/* --------------- Definition --------------- */

struct ToEnd {};
template <typename T> class Option {
  bool is_some_;
  union {
    T value_;
  };

public:
  Option() : is_some_(false) {}
  Option(T &&value) : is_some_(true), value_(std::move(value)) {}
  ~Option() {
    if (is_some_)
      value_.~T();
  }
  [[nodiscard]] explicit operator bool() const { return is_some_; }
  [[nodiscard]] operator T() && { return std::move(value_); }
  // [[nodiscard]] operator T() const & { return value_; }
  [[nodiscard]] T clone() const & { return value_; }
};

class StringView {
  const char *ptr_;
  size_t len_;

public:
  StringView();
  StringView(const char *ptr, size_t len);
  StringView(const StringView &other);
  StringView(const char *ptr);
  StringView &operator=(const StringView &other);
  [[nodiscard]] bool operator==(StringView other) const;

  [[nodiscard]] Option<const char *> get(size_t i) const;
  [[nodiscard]] const char &operator[](size_t i) const;
  [[nodiscard]] Option<StringView> get(size_t i, size_t j) const;
  [[nodiscard]] StringView operator()(size_t i, size_t j) const;
  [[nodiscard]] Option<StringView> get(size_t i, ToEnd) const;
  [[nodiscard]] StringView operator()(size_t i, ToEnd) const;

  [[nodiscard]] const char *ptr() const;
  [[nodiscard]] size_t len() const;
  [[nodiscard]] const char *begin() const;
  [[nodiscard]] const char *end() const;
  [[nodiscard]] const char *rbegin() const;
  [[nodiscard]] const char *rend() const;
  [[nodiscard]] bool is_empty() const;
  [[nodiscard]] bool is_valid() const;
  [[nodiscard]] explicit operator bool() const;

  [[nodiscard]] StringView trim_start() const;
  [[nodiscard]] StringView trim_end() const;
  [[nodiscard]] StringView trim() const;

  /**
   * @warning Checks individual characters, for removing prefixes use
   * trim_prefix
   */
  [[nodiscard]] StringView trim_start_matches(StringView chars) const;
  [[nodiscard]] StringView
  trim_start_matches(std::function<bool(char)> predicate) const;

  /**
   * @warning Checks individual characters, for removing suffixes use
   * trim_suffix
   */
  [[nodiscard]] StringView trim_end_matches(StringView chars) const;
  [[nodiscard]] StringView
  trim_end_matches(std::function<bool(char)> predicate) const;

  /**
   * @warning Checks individual characters, for removing prefixes and suffixes
   * use trim_prefix and trim_suffix
   */
  [[nodiscard]] StringView trim_matches(StringView chars) const;
  [[nodiscard]] StringView
  trim_matches(std::function<bool(char)> predicate) const;

  [[nodiscard]] StringView trim_prefix(StringView prefix) const;
  [[nodiscard]] StringView trim_suffix(StringView suffix) const;

  [[nodiscard]] Option<size_t> find(StringView pat) const;
  [[nodiscard]] Option<size_t> find(std::function<bool(char)> predicate) const;
  [[nodiscard]] Option<size_t> rfind(StringView pat) const;
  [[nodiscard]] Option<size_t> rfind(std::function<bool(char)> predicate) const;

  [[nodiscard]] bool contains(StringView pat) const;
  [[nodiscard]] bool starts_with(StringView pat) const;
  [[nodiscard]] bool ends_with(StringView pat) const;
};

#ifdef EZSTR_IMPLEMENTATION

/* --------------- Implementation --------------- */

/* --------------- Ctors + Operators --------------- */
StringView::StringView() : ptr_(nullptr) {}
StringView::StringView(const char *ptr, size_t len) : ptr_(ptr), len_(len) {}
StringView::StringView(const StringView &other)
    : ptr_(other.ptr_), len_(other.len_) {}
StringView::StringView(const char *ptr) : ptr_(ptr), len_(strlen(ptr)) {}
StringView &StringView::operator=(const StringView &other) {
  ptr_ = other.ptr_;
  len_ = other.len_;
  return *this;
}
bool StringView::operator==(StringView other) const {
  if (len() != other.len())
    return false;
  for (size_t i = 0; i < len(); i++)
    if (ptr()[i] != other[i])
      return false;
  return true;
}
StringView::operator bool() const { return !is_empty() && is_valid(); }
const char &StringView::operator[](size_t i) const { return ptr()[i]; }
StringView StringView::operator()(size_t i, size_t j) const {
  return StringView(ptr() + i, j - i);
}
StringView StringView::operator()(size_t i, ToEnd) const {
  return (*this)(i, len());
}

/* --------------- Getters + KindaGetters --------------- */
const char *StringView::ptr() const { return ptr_; }
size_t StringView::len() const { return len_; }
const char *StringView::begin() const { return ptr(); }
const char *StringView::end() const { return ptr() + len(); }
const char *StringView::rbegin() const { return end(); }
const char *StringView::rend() const { return begin(); }
bool StringView::is_empty() const { return len() == 0; }
bool StringView::is_valid() const { return ptr(); }
Option<const char *> StringView::get(size_t i) const {
  return i < len() ? Option(&ptr()[i]) : Option<const char *>();
}
Option<StringView> StringView::get(size_t i, size_t j) const {
  return j <= len() && i <= j ? Option((*this)(i, j)) : Option<StringView>();
}
Option<StringView> StringView::get(size_t i, ToEnd) const {
  return i <= len() ? Option((*this)(i, ToEnd{})) : Option<StringView>();
}

/* --------------- Trim --------------- */
StringView StringView::trim_start() const {
  return trim_start_matches(
      [](char c) { return isspace(static_cast<unsigned char>(c)); });
}
StringView StringView::trim_end() const {
  return trim_end_matches(
      [](char c) { return isspace(static_cast<unsigned char>(c)); });
}
StringView StringView::trim() const { return trim_start().trim_end(); }
StringView
StringView::trim_start_matches(std::function<bool(char)> predicate) const {
  StringView newSV(*this);
  while (newSV.len() && predicate(newSV[0])) {
    newSV.ptr_++;
    newSV.len_--;
  }
  return newSV;
}
StringView StringView::trim_start_matches(StringView chars) const {
  return trim_start_matches([chars](char c) {
    for (char ch : chars)
      if (c == ch)
        return true;
    return false;
  });
}
StringView
StringView::trim_end_matches(std::function<bool(char)> predicate) const {
  StringView newSV(*this);
  while (newSV.len() && predicate(newSV[newSV.len() - 1]))
    newSV.len_--;
  return newSV;
}
StringView StringView::trim_end_matches(StringView chars) const {
  return trim_end_matches([chars](char c) {
    for (char ch : chars)
      if (c == ch)
        return true;
    return false;
  });
}
StringView StringView::trim_matches(std::function<bool(char)> predicate) const {
  return trim_start_matches(predicate).trim_end_matches(predicate);
}
StringView StringView::trim_matches(StringView chars) const {
  return trim_start_matches(chars).trim_end_matches(chars);
}
StringView StringView::trim_prefix(StringView prefix) const {
  StringView newSV(*this);
  if (newSV.starts_with(prefix)) {
    newSV.ptr_ += prefix.len();
    newSV.len_ -= prefix.len();
  }
  return newSV;
}
StringView StringView::trim_suffix(StringView suffix) const {
  StringView newSV(*this);
  if (newSV.ends_with(suffix))
    newSV.len_ -= suffix.len();
  return newSV;
}

/* --------------- Find --------------- */
Option<size_t> StringView::find(StringView pat) const {
  StringView copy = *this;
  while (pat.len() <= copy.len()) {
    if (copy.starts_with(pat))
      return len() - copy.len();
    copy.ptr_++;
    copy.len_--;
  }
  return Option<size_t>();
}
Option<size_t> StringView::find(std::function<bool(char)> predicate) const {
  size_t i = 0;
  for (char c : (*this)) {
    if (predicate(c))
      return i;
    i++;
  }
  return Option<size_t>();
}
Option<size_t> StringView::rfind(StringView pat) const {
  StringView copy = *this;
  while (pat.len() <= copy.len()) {
    if (copy.ends_with(pat))
      return copy.len() - pat.len();
    copy.len_--;
  }
  return Option<size_t>();
}
Option<size_t> StringView::rfind(std::function<bool(char)> predicate) const {
  size_t i = len() - 1;
  for (auto c = rbegin(); c != rend(); c++) {
    if (predicate(*c))
      return i;
    i--;
  }
  return Option<size_t>();
}

/* --------------- Pattern Matching --------------- */
bool StringView::contains(StringView pat) const {
  if (pat.len() > len())
    return false;
  for (size_t i = 0; i <= len() - pat.len(); i++)
    if (pat == StringView(ptr() + i, pat.len()))
      return true;
  return false;
}
bool StringView::starts_with(StringView pat) const {
  return pat.len() <= len() && pat == StringView(ptr(), pat.len());
}
bool StringView::ends_with(StringView pat) const {
  return pat.len() <= len() &&
         pat == StringView(ptr() + len() - pat.len(), pat.len());
}

#endif // EZSTR_IMPLEMENTATION
} // namespace ezstr
#endif // !EZSTR_H
