#include <cctype>
#include <cstddef>
#include <cstring>

#define DEV 1
#if DEV
#define EZSTR_IMPL
#endif

#ifndef EZSTR_H
#define EZSTR_H
namespace ezstr {

/* --------------- Definition --------------- */

class StringView {
public:
  StringView(char *ptr, size_t len);
  StringView(StringView &&other);
  StringView(const StringView &other);
  StringView(char *ptr);
  StringView &operator=(StringView &&other);
  StringView &operator=(const StringView &other);
  [[nodiscard]] bool operator==(const StringView &other) const;
  [[nodiscard]] bool operator==(const char *other) const;

  [[nodiscard]] char *ptr() const;
  [[nodiscard]] size_t len() const;

  [[nodiscard]] StringView trim_left() const;
  [[nodiscard]] StringView trim_right() const;
  [[nodiscard]] StringView trim() const;

  [[nodiscard]] bool contains(const StringView &pat);
  [[nodiscard]] bool contains(const char *pat);
  [[nodiscard]] bool starts_with(const StringView &pat);
  [[nodiscard]] bool starts_with(const char *pat);
  [[nodiscard]] bool ends_with(const StringView &pat);
  [[nodiscard]] bool ends_with(const char *pat);

private:
  char *ptr_;
  size_t len_;
};

#ifdef EZSTR_IMPL

/* --------------- Implementation --------------- */

/* --------------- Ctors + Operators --------------- */
StringView::StringView(char *ptr, size_t len) : ptr_(ptr), len_(len) {}
StringView::StringView(StringView &&other)
    : ptr_(other.ptr()), len_(other.len()) {}
StringView::StringView(const StringView &other)
    : ptr_(other.ptr_), len_(other.len_) {}
StringView::StringView(char *ptr) : ptr_(ptr), len_(strlen(ptr)) {}
StringView &StringView::operator=(StringView &&other) {
  ptr_ = other.ptr();
  len_ = other.len();
  return *this;
}
StringView &StringView::operator=(const StringView &other) {
  ptr_ = other.ptr_;
  len_ = other.len_;
  return *this;
}
bool StringView::operator==(const StringView &other) const {
  if (len() != other.len())
    return false;
  for (size_t i = 0; i < len(); i++) {
    if (ptr()[i] != other.ptr()[i])
      return false;
  }
  return true;
}
bool StringView::operator==(const char *other) const {
  return *this == StringView(const_cast<char *>(other));
}

/* --------------- Getters --------------- */
char *StringView::ptr() const { return ptr_; }
size_t StringView::len() const { return len_; }

/* --------------- Trim --------------- */
StringView StringView::trim_left() const {
  StringView newSV(*this);
  while (newSV.len() && isspace(static_cast<unsigned char>(*newSV.ptr()))) {
    newSV.ptr_++;
    newSV.len_--;
  }
  return newSV;
}
StringView StringView::trim_right() const {
  StringView newSV(*this);
  while (newSV.len() &&
         isspace(static_cast<unsigned char>(newSV.ptr()[newSV.len() - 1]))) {
    newSV.len_--;
  }
  return newSV;
}
StringView StringView::trim() const { return trim_left().trim_right(); }

/* --------------- Pattern Matching --------------- */
#error todo
bool contains(const StringView &pat);
bool contains(const char *pat);
bool starts_with(const StringView &pat);
bool starts_with(const char *pat);
bool ends_with(const StringView &pat);
bool ends_with(const char *pat);

#endif // EZSTR_IMPL
} // namespace ezstr
#endif // !EZSTR_H
