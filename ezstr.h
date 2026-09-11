#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
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

/* --------------- TODO ---------------
 * Replace
 * trim_end_mut
 * trim_end_matches_mut
 * trim_suffix_mut
 */

/* --------------- Definition + Templates --------------- */

class String;
class StringView;
class SplitWhitespace;
namespace {
template <typename T> class Option;
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
  [[nodiscard]] T unwrap_or(T &&substitute) && {
    if (is_some_)
      return std::move(value_);
    else
      return substitute;
  }
  [[nodiscard]] T unwrap_or_else(std::function<T()> fn) && {
    if (is_some_)
      return std::move(value_);
    else
      return fn();
  }
  [[nodiscard]] T clone() const & { return value_; }
};
} // namespace

class StringView {
  const char *ptr_;
  size_t len_;

public:
  StringView();
  StringView(const char *ptr, size_t len);
  StringView(const StringView &other);
  StringView(const char *ptr);
  StringView &operator=(const StringView &other);
  [[nodiscard]] bool operator==(const String &other) const;
  [[nodiscard]] bool operator==(const StringView other) const;

  [[nodiscard]] Option<const char *> get(size_t i) const;
  [[nodiscard]] const char &operator[](size_t i) const;
  [[nodiscard]] Option<StringView> get(size_t i, size_t j) const;
  [[nodiscard]] StringView operator()(size_t i, size_t j) const;

  void unsafe_set_ptr(const char *ptr);
  void unsafe_set_len(size_t len);
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

  [[nodiscard]] SplitWhitespace split_whitespace() const;
  [[nodiscard]] String to_string() const;
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
  String(StringView sv);
  String(String &&str);
  ~String();
  const StringView *operator->() const;
  const StringView &operator*() const;
  String &operator=(const StringView &other);
  String &operator=(String &&other);
  String &operator+=(const StringView &other);
  String &operator+=(const String &other);
  [[nodiscard]] bool operator==(const StringView &other) const;
  [[nodiscard]] bool operator==(const String &other) const;
  [[nodiscard]] StringView as_str() const;
  [[nodiscard]] size_t cap() const;
  [[nodiscard]] char *ptr_mut() const;
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
/**
 * @warning
 * Use after free for self assignment
 * e.g. s += s or s += s.as_str();
 */
String &String::operator+=(const StringView &other) {
  assert(ptr_mut() != other.ptr());
  size_t old_len = as_str().len();
  size_t len = old_len + other.len();
  if (len > cap()) {
    cap_ = 2 * (len);
    char *ptr = (char *)malloc(cap() + 1);
    memcpy(ptr, as_str().ptr(), old_len);
    free(ptr_mut());
    sv_ = {ptr, len};
  }
  // other.ptr() might be freed if this == other
  memcpy(ptr_mut() + old_len, other.ptr(), other.len());
  ptr_mut()[len] = '\0';
  return *this;
}
/**
 * @warning
 * Use after free for self assignment
 * e.g. s += s or s += s.as_str();
 */
String &String::operator+=(const String &other) {
  return (*this) += other.as_str();
}
String::String(String &&str) : sv_(str.sv_), cap_(str.cap_) { str.sv_ = {}; }
String &String::operator=(const StringView &other) {
  if (other.len() > cap()) {
    free(ptr_mut());
    cap_ = other.len();
    sv_ = {(char *)malloc(cap() + 1), cap()};
  }
  memcpy(ptr_mut(), other.ptr(), other.len());
  ptr_mut()[other.len()] = '\0';
  return *this;
}
/**
 * @warning
 * Pointer of the returned string is freed for self assignment
 * e.g. s = std::move(s);
 */
String &String::operator=(String &&other) {
  assert(ptr_mut() != other->ptr());
  free(ptr_mut());
  sv_ = other.sv_;
  cap_ = other.cap();
  other.sv_ = {};
  return *this;
}
String::String() : cap_(INIT_CAP) {
  char *ptr = (char *)malloc(INIT_CAP + 1);
  ptr[0] = '\0';
  sv_ = {ptr, 0};
}
String::String(StringView sv)
    : cap_(INIT_CAP > sv.len() ? INIT_CAP : sv.len()) {
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
Option<const char *> StringView::get(size_t i) const {
  return i < len() ? Option(&ptr()[i]) : Option<const char *>();
}
Option<StringView> StringView::get(size_t i, size_t j) const {
  return j <= len() && i <= j ? Option((*this)(i, j)) : Option<StringView>();
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
StringView String::as_str() const { return sv_; }
size_t String::cap() const { return cap_; }
void StringView::unsafe_set_ptr(const char *ptr) { ptr_ = ptr; }
void StringView::unsafe_set_len(size_t len) { len_ = len; }

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

// Cleaner implementation possible using slices
bool StringView::starts_with(StringView pat) const {
  return pat.len() <= len() && pat == StringView(ptr(), pat.len());
}
// Cleaner implementation possible using slices
bool StringView::ends_with(StringView pat) const {
  return pat.len() <= len() &&
         pat == StringView(ptr() + len() - pat.len(), pat.len());
}

/* --------------- Split --------------- */
SplitWhitespace StringView::split_whitespace() const {
  return SplitWhitespace(*this);
}

#endif // EZSTR_IMPLEMENTATION
} // namespace ezstr
#endif // !EZSTR_H
