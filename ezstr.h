#include <cctype>
#include <cstddef>
#include <cstring>
#include <functional>

#define DEV 1
#if DEV
#define EZSTR_IMPL
#endif

#ifndef EZSTR_H
#define EZSTR_H
namespace ezstr {

/* --------------- TODO --------------- */
/* Split
 * Find
 * Asserts
 */

/* --------------- Definition --------------- */

struct ToEnd {};

class StringView {
public:
  StringView(const char *ptr, size_t len);
  StringView(const StringView &other);
  StringView(const char *ptr);
  StringView &operator=(const StringView &other);
  [[nodiscard]] bool operator==(StringView other) const;

  /**
   * @warning returns NULL if the index is out of range
   */
  [[nodiscard]] const char *get(size_t i) const;
  [[nodiscard]] const char &operator[](size_t i) const;
  /**
   * @warning return value !is_valid() for incorrect parameters
   */
  [[nodiscard]] StringView get(size_t i, size_t j) const;
  [[nodiscard]] StringView operator()(size_t i, size_t j) const;
  /**
   * @warning return value !is_valid() for incorrect *i* parameter
   */
  [[nodiscard]] StringView get(size_t i, ToEnd) const;
  [[nodiscard]] StringView operator()(size_t i, ToEnd) const;

  [[nodiscard]] const char *ptr() const;
  [[nodiscard]] size_t len() const;
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

  [[nodiscard]] bool contains(StringView pat) const;
  [[nodiscard]] bool starts_with(StringView pat) const;
  [[nodiscard]] bool ends_with(StringView pat) const;

private:
  const char *ptr_;
  size_t len_;
};

#ifdef EZSTR_IMPL

/* --------------- Implementation --------------- */

/* --------------- Ctors + Operators --------------- */
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
  for (size_t i = 0; i < len(); i++) {
    if (ptr()[i] != other[i])
      return false;
  }
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
bool StringView::is_empty() const { return len() == 0; }
bool StringView::is_valid() const { return ptr(); }
const char *StringView::get(size_t i) const {
  return i < len() ? &ptr()[i] : nullptr;
}
StringView StringView::get(size_t i, size_t j) const {
  return j <= len() && i <= j ? (*this)(i, j) : StringView(nullptr, 0);
}
StringView StringView::get(size_t i, ToEnd) const {
  return i <= len() ? (*this)(i, ToEnd{}) : StringView(nullptr, 0);
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
    for (size_t i = 0; i < chars.len(); i++)
      if (c == chars[i])
        return true;
    return false;
  });
}
StringView
StringView::trim_end_matches(std::function<bool(char)> predicate) const {
  StringView newSV(*this);
  while (newSV.len() && predicate(newSV[newSV.len() - 1])) {
    newSV.len_--;
  }
  return newSV;
}
StringView StringView::trim_end_matches(StringView chars) const {
  return trim_end_matches([chars](char c) {
    for (size_t i = 0; i < chars.len(); i++)
      if (c == chars[i])
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

#endif // EZSTR_IMPL
} // namespace ezstr
#endif // !EZSTR_H
