#include <cstddef>
#include <cstdint>
#include <cstdlib>

#define STRINGRS_DEV 1
#if STRINGRS_DEV
#define STRINGRS_IMPLEMENTATION
#endif // STRINGRS_DEV

#ifndef STRINGRS_H
#define STRINGRS_H

#define STRING_FMT_LITERAL "%.*s"
#define STRING_FMT_VALUES(str) (str.len()), (str)
#define INITIAL_CAPACITY 4

namespace stringrs {

// Definitions

uint8_t byte_char_length(uint8_t first_byte);
bool is_valid_utf8(char *ptr);

class Strrs {
public:
  Strrs(char *ptr, size_t len);
  char *ptr();
  size_t len();

private:
  char *ptr_;
  size_t len_;
};

class Stringrs {
public:
  Stringrs();
  //  Stringrs(Stringrs &&other);
  //  Stringrs(const Stringrs &other);
  //  Stringrs &operator=(Stringrs &&other);
  //  Stringrs &operator=(const Stringrs &);
  //  ~Stringrs();
  size_t capacity();
  Strrs as_str();

private:
  Strrs str_;
  size_t cap_;
};

#ifdef STRINGRS_IMPLEMENTATION

// Implementations

/**
 * @returns Expected UTF-8 length based on the parameter
 * @warning Returns 0 on if the parameter is not a valid UTF-8 starting byte
 */
uint8_t byte_char_length(uint8_t first_byte) {
  if (!(first_byte >> 7))
    return 1;
  if (!(first_byte & (1 << 6)))
    return 0;
  if (!(first_byte & (1 << 5)))
    return 2;
  if (!(first_byte & (1 << 4)))
    return 3;
  if (!(first_byte & (1 << 3)))
    return 4;
  return 0;
}

bool is_valid_utf8(char *ptr) {
  while (*ptr) {
    uint8_t char_len = byte_char_length(*ptr);
    if (!char_len)
      return false;
    if (char_len == 1) {
      ptr++;
      continue;
    }

    uint8_t significant_bits = 3 + 4 - char_len;
    uint32_t char_val = (*ptr++) & ((1 << significant_bits) - 1);
    for (uint8_t i = 1; i < char_len; i++, ptr++) {
      uint8_t byte = (uint8_t)*ptr;
      if (!byte || (byte >> 6) != 0b10)
        return false;
      char_val = char_val << 6;
      char_val += (255 >> 2) & (byte);
    }
    uint32_t low_bound, up_bound;
    switch (char_len) {
    case 2:
      low_bound = 0x80;
      up_bound = 0x7FF;
      break;
    case 3:
      low_bound = 0x800;
      up_bound = 0xFFFF;
      break;
    case 4:
      low_bound = 0x10000;
      up_bound = 0x10FFFF;
      break;
    }
    if (!(low_bound <= char_val && char_val <= up_bound))
      return false;
    if (0xD800 <= char_val && char_val <= 0xDFFF)
      return false;
  }
  return true;
}

char *Strrs::ptr() { return ptr_; }

size_t Strrs::len() { return len_; }

size_t Stringrs::capacity() { return cap_; }

Strrs Stringrs::as_str() { return str_; }

Strrs::Strrs(char *ptr, size_t len) : ptr_(ptr), len_(len) {}

Stringrs::Stringrs()
    : str_((char *)malloc(INITIAL_CAPACITY), 0), cap_(INITIAL_CAPACITY) {}

#endif // STRINGRS_IMPLEMENTATION
} // namespace stringrs
#endif // !STRINGRS_H
