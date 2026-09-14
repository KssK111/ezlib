#include <functional>
#include <utility>

#ifndef EZOPTION_H
#define EZOPTION_H
namespace ezopt {

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
    return is_some_ ? std::move(value_) : std::move(substitute);
  }
  [[nodiscard]] T unwrap_or_else(std::function<T()> fn) && {
    return is_some_ ? std::move(value_) : fn();
  }
};

} // namespace ezopt
#endif // !EZOPTION_H
