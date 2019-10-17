#ifndef INCLUDE_GUARD_4A0FC485_1D1B_410C_946B_357D4B046866
#define INCLUDE_GUARD_4A0FC485_1D1B_410C_946B_357D4B046866

#include <iosfwd>
#include <string>
#include <variant>
#include "result.hpp"

namespace chfs {
namespace error {
class Message {
  static const std::string quote;
  static const std::string colon;
  static const std::string error;
  std::string str;

 public:
  Message() = default;
  explicit Message(const std::string &msg);
  Message(const std::string &msg, const std::string &desc);
  Message(const std::string &msg, int desc);
  void print() const;
};

struct Help {
  const Message msg;
  Help();
};
struct UnknownOption {
  const Message msg;
  explicit UnknownOption(const std::string &option);
};
struct InvalidOption {
  const Message msg;
  explicit InvalidOption(const std::string &option);
};
struct MissingOption {
  const Message msg;
  explicit MissingOption(const std::string &param);
};
struct MissingParam {
  const Message msg;
  explicit MissingParam(const std::string &option);
};
struct NoInput {
  const Message msg;
  NoInput();
};
struct OpenFailure {
  const Message msg;
  explicit OpenFailure(const std::string &name);
};
struct InvalidInput {
  const Message msg;
  explicit InvalidInput(int row);
};
struct ProcessFailure {
  const Message msg;
  ProcessFailure();
};
}  // namespace error

using Error =
    std::variant<error::Help, error::UnknownOption, error::InvalidOption,
                 error::MissingOption, error::MissingParam, error::NoInput,
                 error::OpenFailure, error::InvalidInput,
                 error::ProcessFailure>;

template <typename T>
using Result = result::Result<T, Error>;
using result::ok, result::err;

template <typename E, typename... U>
auto emit_error(U &&... u) {
  return err(Error{std::in_place_type<E>, std::forward<U>(u)...});
}

int print_error(const Error &error, const std::string &cmd);
template <typename T>
int print_error(const Result<T> &result, const std::string &cmd) {
  if (result) {
    return 0;
  }
  return print_error(result.error(), cmd);
}

}  // namespace chfs

#endif  // INCLUDE_GUARD_4A0FC485_1D1B_410C_946B_357D4B046866
