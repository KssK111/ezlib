#ifndef EZSTR_H
#define EZSTR_H

#include "ezoption.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <utility>

// Define if you are sure, your system has certain functionality
// #define EZSTR_HAS_MEMMEM

#ifdef _GNU_SOURCE
#ifndef EZSTR_HAS_MEMMEM
#define EZSTR_HAS_MEMMEM
#endif // !EZSTR_HAS_MEMMEM
#endif // _GNU_SOURCE

#define EZSTR_DEV 0
#if EZSTR_DEV
#define EZSTR_IMPLEMENTATION
#endif

/* --------------- TODO ---------------
 * ?StringView::replace(predicate, char)
 * ?String::unsafe_replace_mut(old, new)
 */

/* --------------- Declaration + Templates --------------- */
namespace ezstr {

class String;
class StringView;
class SplitWhitespace;

class StringView {
  const char *ptr_;
  size_t len_;

public:
  StringView();
  StringView(const char *ptr, size_t len);
  StringView(const StringView &other);
  StringView(const char *ptr);
  StringView(char &c);
  [[nodiscard]] String to_string() const;
  StringView &operator=(const StringView &other);
  [[nodiscard]] bool operator==(const String &other) const;
  [[nodiscard]] bool operator==(const StringView other) const;

  [[nodiscard]] ezopt::Option<const char *> get(size_t i) const;
  [[nodiscard]] const char &operator[](size_t i) const;
  [[nodiscard]] ezopt::Option<StringView> get(size_t i, size_t j) const;
  [[nodiscard]] StringView operator()(size_t i, size_t j) const;

  void unsafe_set_ptr(const char *ptr);
  void unsafe_set_len(size_t len);
  [[nodiscard]] char *unsafe_ptr_mut() const;
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
   * @warning
   * Checks individual characters, for removing prefixes use trim_prefix
   */
  [[nodiscard]] StringView trim_start_matches(StringView chars) const;
  [[nodiscard]] StringView
  trim_start_matches(std::function<bool(char)> predicate) const;

  /**
   * @warning
   * Checks individual characters, for removing suffixes use trim_suffix
   */
  [[nodiscard]] StringView trim_end_matches(StringView chars) const;
  [[nodiscard]] StringView
  trim_end_matches(std::function<bool(char)> predicate) const;

  /**
   * @warning
   * Checks individual characters, for removing prefixes and suffixes use
   * trim_prefix and trim_suffix
   */
  [[nodiscard]] StringView trim_matches(StringView chars) const;
  [[nodiscard]] StringView
  trim_matches(std::function<bool(char)> predicate) const;

  [[nodiscard]] StringView trim_prefix(StringView prefix) const;
  [[nodiscard]] StringView trim_suffix(StringView suffix) const;

  [[nodiscard]] ezopt::Option<size_t> find(StringView pat) const;
  [[nodiscard]] ezopt::Option<size_t>
  find(std::function<bool(char)> predicate) const;
  [[nodiscard]] ezopt::Option<size_t> rfind(StringView pat) const;
  [[nodiscard]] ezopt::Option<size_t>
  rfind(std::function<bool(char)> predicate) const;

  [[nodiscard]] bool contains(StringView pat) const;
  [[nodiscard]] bool starts_with(StringView pat) const;
  [[nodiscard]] bool ends_with(StringView pat) const;

  [[nodiscard]] SplitWhitespace split_whitespace() const;

  /**
   * @warning
   * Loops infinitely if from == ""
   */
  [[nodiscard]] String replace(StringView from, StringView to) const;
};

class SplitWhitespace {
  StringView sv_;
  class Iterator {
    StringView sv_;

  public:
    explicit Iterator(StringView sv);
    bool operator!=(const Iterator &other) const;
    bool operator==(const Iterator &other) const;
    Iterator &operator++();
    StringView operator*() const;
  };

public:
  explicit SplitWhitespace(StringView sv);
  Iterator begin() const;
  Iterator end() const;
};

class String {
  static constexpr size_t INIT_CAP = 23;
  StringView sv_;
  size_t cap_;

public:
  String();
  String(size_t cap);
  String(StringView sv);
  String(String &&str);
  String(const char *ptr, size_t len, size_t cap);
  ~String();

  const StringView *operator->() const;
  const StringView &operator*() const;

  /**
   * @warning
   * Pointer of the returned string is freed for self assignment
   * e.g. s = std::move(s);
   */
  String &operator=(String &&other);
  String &operator=(const StringView &other);

  /**
   * @warning
   * Use after free for self assignment
   * e.g. s += s or s += s.as_str();
   */
  String &operator+=(const StringView &other);

  /**
   * @warning
   * Use after free for self assignment
   * e.g. s += s or s += s.as_str();
   */
  String &operator+=(const String &other);

  [[nodiscard]] char &operator[](size_t i) const;
  [[nodiscard]] StringView operator()(size_t i, size_t j) const;
  [[nodiscard]] bool operator==(const StringView &other) const;
  [[nodiscard]] bool operator==(const String &other) const;
  [[nodiscard]] String clone() const;
  [[nodiscard]] StringView as_str() const;
  [[nodiscard]] size_t cap() const;
  [[nodiscard]] char *ptr_mut() const;
  [[nodiscard]] char *move_ptr();
  void unsafe_set_cap(size_t cap);
  void unsafe_set_ptr(const char *ptr);
  void unsafe_set_len(size_t len);
  void unsafe_set_sv(StringView sv);

  /**
   * @warning
   * Checks individual characters, for removing suffixes use trim_suffix_mut
   */
  void trim_end_matches_mut(StringView chars);
  void trim_end_mut();
  void trim_end_matches_mut(std::function<bool(char)> predicate);
  void trim_suffix_mut(StringView suffix);
};

} // namespace ezstr
#endif // !EZSTR_H
#ifdef EZSTR_IMPLEMENTATION
namespace ezstr {

/* --------------- Implementation --------------- */

/* --------------- Ctors + Operators --------------- */
StringView::StringView() : ptr_(nullptr) {}
StringView::StringView(const char *ptr, size_t len) : ptr_(ptr), len_(len) {}
StringView::StringView(const StringView &other)
    : ptr_(other.ptr_), len_(other.len_) {}
StringView::StringView(const char *ptr) : ptr_(ptr), len_(strlen(ptr)) {}
StringView::StringView(char &c) : ptr_(&c), len_(1) {}
StringView &StringView::operator=(const StringView &other) {
  ptr_ = other.ptr_;
  len_ = other.len_;
  return *this;
}
bool StringView::operator==(StringView other) const {
  return len() == other.len() && memcmp(ptr(), other.ptr(), len()) == 0;
}
StringView::operator bool() const { return !is_empty() && is_valid(); }
const char &StringView::operator[](size_t i) const { return ptr()[i]; }
StringView StringView::operator()(size_t i, size_t j) const {
  return StringView(ptr() + i, j - i);
}
SplitWhitespace::SplitWhitespace(StringView sv) : sv_(sv) {}
SplitWhitespace::Iterator::Iterator(StringView sv) : sv_(sv) {}
bool SplitWhitespace::Iterator::operator!=(const Iterator &other) const {
  return sv_.ptr() != other.sv_.ptr();
}
bool SplitWhitespace::Iterator::operator==(const Iterator &other) const {
  return sv_.ptr() == other.sv_.ptr();
}
SplitWhitespace::Iterator &SplitWhitespace::Iterator::operator++() {
  sv_ = sv_.trim_start_matches(
               [](char c) { return !isspace(static_cast<unsigned char>(c)); })
            .trim_start();
  return *this;
}
StringView SplitWhitespace::Iterator::operator*() const {
  StringView copy = sv_.trim_start();
  return copy(0, copy.find([](char c) {
                       return isspace(static_cast<unsigned char>(c));
                     })
                     .unwrap_or(copy.len()));
}
String &String::operator+=(const StringView &other) {
  assert(ptr_mut() != other.ptr());
  size_t old_len = as_str().len();
  size_t len = old_len + other.len();
  if (len > cap()) {
    cap_ = 2 * (len);
    // used realloc for simplicity, but previos solution MAY be faster
    unsafe_set_ptr((char *)realloc(ptr_mut(), cap() + 1));
  }
  unsafe_set_len(len);
  // other.ptr() might be freed if this == other
  memcpy(ptr_mut() + old_len, other.ptr(), other.len());
  ptr_mut()[len] = '\0';
  return *this;
}
String &String::operator+=(const String &other) {
  return *this += other.as_str();
}
String::String(String &&str) : sv_(str.sv_), cap_(str.cap_) {
  str.unsafe_set_ptr(nullptr);
}
String &String::operator=(const StringView &other) {
  if (other.len() > cap()) {
    // realloc is not a good simplification of the code here
    // unnecessary memcpy - always, no new allocation - sometimes
    free(ptr_mut());
    cap_ = other.len();
    unsafe_set_ptr((char *)malloc(cap() + 1));
  }
  unsafe_set_len(other.len());
  memcpy(ptr_mut(), other.ptr(), other.len());
  ptr_mut()[other.len()] = '\0';
  return *this;
}
String::String(const char *ptr, size_t len, size_t cap)
    : sv_(ptr, len), cap_(cap) {}
String &String::operator=(String &&other) {
  assert(ptr_mut() != other->ptr());
  free(ptr_mut());
  sv_ = other.sv_;
  cap_ = other.cap();
  other.unsafe_set_ptr(nullptr);
  return *this;
}
String::String() : String(INIT_CAP) {}
String::String(size_t cap) : cap_(std::max(cap, INIT_CAP)) {
  char *ptr = (char *)malloc(cap_ + 1);
  ptr[0] = '\0';
  sv_ = {ptr, 0};
}
String::String(StringView sv) : cap_(std::max(sv.len(), INIT_CAP)) {
  char *ptr = (char *)malloc(cap() + 1);
  memcpy(ptr, sv.ptr(), sv.len());
  ptr[sv.len()] = '\0';
  sv_ = {ptr, sv.len()};
}
String::~String() { free((void *)sv_.ptr()); }
const StringView *String::operator->() const { return &sv_; }
const StringView &String::operator*() const { return sv_; }
bool StringView::operator==(const String &other) const {
  return *this == other.as_str();
}
char &String::operator[](size_t i) const { return const_cast<char &>(sv_[i]); }
StringView String::operator()(size_t i, size_t j) const { return sv_(i, j); }
bool String::operator==(const StringView &other) const {
  return as_str() == other;
}
bool String::operator==(const String &other) const {
  return as_str() == other.as_str();
}
String StringView::to_string() const { return String(*this); }

/* --------------- Getters + KindaGetters + Setters --------------- */
const char *StringView::ptr() const { return ptr_; }
size_t StringView::len() const { return len_; }
const char *StringView::begin() const { return ptr(); }
const char *StringView::end() const { return ptr() + len(); }
const char *StringView::rbegin() const { return end(); }
const char *StringView::rend() const { return begin(); }
bool StringView::is_empty() const { return len() == 0; }
bool StringView::is_valid() const { return ptr(); }
ezopt::Option<const char *> StringView::get(size_t i) const {
  return i < len() ? ezopt::Option(&ptr()[i]) : ezopt::Option<const char *>();
}
ezopt::Option<StringView> StringView::get(size_t i, size_t j) const {
  return j <= len() && i <= j ? ezopt::Option((*this)(i, j))
                              : ezopt::Option<StringView>();
}
SplitWhitespace::Iterator SplitWhitespace::begin() const {
  return SplitWhitespace::Iterator(sv_.trim_start());
}
SplitWhitespace::Iterator SplitWhitespace::end() const {
  return SplitWhitespace::Iterator(
      StringView(sv_.ptr() + sv_.trim_end().len(), 0));
}
constexpr size_t String::INIT_CAP;
char *String::ptr_mut() const { return const_cast<char *>((*this)->ptr()); }
char *String::move_ptr() {
  char *ptr = ptr_mut();
  this->sv_.unsafe_set_ptr(nullptr);
  return ptr;
}
String String::clone() const { return String(as_str()); }
StringView String::as_str() const { return sv_; }
size_t String::cap() const { return cap_; }
void StringView::unsafe_set_ptr(const char *ptr) { ptr_ = ptr; }
void StringView::unsafe_set_len(size_t len) { len_ = len; }
char *StringView::unsafe_ptr_mut() const { return const_cast<char *>(ptr()); }
void String::unsafe_set_cap(size_t cap) { cap_ = cap; }
void String::unsafe_set_ptr(const char *ptr) { sv_.unsafe_set_ptr(ptr); }
void String::unsafe_set_len(size_t len) { sv_.unsafe_set_len(len); }
void String::unsafe_set_sv(StringView sv) { sv_ = sv; }

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
  StringView copy(*this);
  while (copy.len() && predicate(copy[0])) {
    copy.ptr_++;
    copy.len_--;
  }
  return copy;
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
  StringView copy(*this);
  while (copy.len() && predicate(copy[copy.len() - 1]))
    copy.len_--;
  return copy;
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
  StringView copy(*this);
  if (copy.starts_with(prefix)) {
    copy.ptr_ += prefix.len();
    copy.len_ -= prefix.len();
  }
  return copy;
}
StringView StringView::trim_suffix(StringView suffix) const {
  StringView copy(*this);
  if (copy.ends_with(suffix))
    copy.len_ -= suffix.len();
  return copy;
}
void String::trim_end_mut() { unsafe_set_len((*this)->trim_end().len()); }
void String::trim_end_matches_mut(std::function<bool(char)> predicate) {
  unsafe_set_len((*this)->trim_end_matches(predicate).len());
}
void String::trim_end_matches_mut(StringView chars) {
  unsafe_set_len((*this)->trim_end_matches(chars).len());
}
void String::trim_suffix_mut(StringView suffix) {
  unsafe_set_len((*this)->trim_suffix(suffix).len());
}

/* --------------- Find --------------- */
ezopt::Option<size_t> StringView::find(StringView pat) const {
#ifdef EZSTR_HAS_MEMMEM
  char *pat_addr = (char *)memmem(ptr(), len(), pat.ptr(), pat.len());
  return pat_addr ? ezopt::Option(static_cast<size_t>(pat_addr - ptr()))
                  : ezopt::Option<size_t>();
#else
  StringView copy = *this;
  while (pat.len() <= copy.len()) {
    if (copy.starts_with(pat))
      return len() - copy.len();
    copy.ptr_++;
    copy.len_--;
  }
  return ezopt::Option<size_t>();
#endif
}
ezopt::Option<size_t>
StringView::find(std::function<bool(char)> predicate) const {
  size_t i = 0;
  for (char c : *this) {
    if (predicate(c))
      return i;
    i++;
  }
  return ezopt::Option<size_t>();
}
ezopt::Option<size_t> StringView::rfind(StringView pat) const {
  StringView copy = *this;
  while (pat.len() <= copy.len()) {
    if (copy.ends_with(pat))
      return copy.len() - pat.len();
    copy.len_--;
  }
  return ezopt::Option<size_t>();
}
ezopt::Option<size_t>
StringView::rfind(std::function<bool(char)> predicate) const {
  size_t i = len() - 1;
  for (auto c = rbegin(); c != rend(); c++) {
    if (predicate(*c))
      return i;
    i--;
  }
  return ezopt::Option<size_t>();
}

/* --------------- Pattern Matching --------------- */
bool StringView::contains(StringView pat) const {
  if (pat.len() > len())
    return false;
#ifdef EZSTR_HAS_MEMMEM
  return memmem(ptr(), len(), pat.ptr(), pat.len()) != nullptr;
#else
  for (size_t i = 0; i <= len() - pat.len(); i++)
    if (pat == StringView(ptr() + i, pat.len()))
      return true;
  return false;
#endif // EZSTR_HAS_MEMMEM
}

bool StringView::starts_with(StringView pat) const {
  return pat.len() <= len() && pat == (*this)(0, pat.len());
}
bool StringView::ends_with(StringView pat) const {
  return pat.len() <= len() && pat == (*this)(len() - pat.len(), len());
}

/* --------------- Split --------------- */
SplitWhitespace StringView::split_whitespace() const {
  return SplitWhitespace(*this);
}

/* --------------- Replace --------------- */
String StringView::replace(StringView from, StringView to) const {
  assert(!(from == ""));
  size_t to_div_from_ceil =
      to.len() / from.len() + (to.len() % from.len() != 0);
  size_t cap = len() * to_div_from_ceil;

  String ret(cap);
  StringView copy = *this;
  while (copy.ptr() != ptr() + len()) {
    size_t from_idx = copy.find(from).unwrap_or(copy.len());
    ret += copy(0, from_idx);
    copy = copy(from_idx, copy.len());
    if (copy.ptr() == ptr() + len())
      break;
    ret += to;
    copy = copy(from.len(), copy.len());
  }
  return ret;
}

} // namespace ezstr
#endif // EZSTR_IMPLEMENTATION
