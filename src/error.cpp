#include "error.hpp"
#include <iostream>
#include "params.hpp"

namespace chfs {
namespace error {
const std::string Message::quote{"'"};
const std::string Message::colon{": "};
const std::string Message::error{"Error: "};
Message::Message(const std::string &msg) : str{error + msg} {}
Message::Message(const std::string &msg, const std::string &desc)
    : str{error + msg + colon + quote + desc + quote} {}
Message::Message(const std::string &msg, int desc)
    : str{error + msg + colon + std::to_string(desc)} {}
void Message::print() const {
  if (!str.empty()) {
    std::cerr << str << std::endl << std::endl;
  }
}

Help::Help() : msg{} {}
UnknownOption::UnknownOption(const std::string &option)
    : msg{"unknown option", option} {}
InvalidOption::InvalidOption(const std::string &option)
    : msg{"invalid option", option} {}
MissingOption::MissingOption(const std::string &param)
    : msg{"missing option before", param} {}
MissingParam::MissingParam(const std::string &option)
    : msg{"missing option after", option} {}
NoInput::NoInput() : msg{"no input file"} {}
OpenFailure::OpenFailure(const std::string &name)
    : msg{"failed to open", name} {}
InvalidInput::InvalidInput(int row) : msg{"invalid cell on line", row} {}
ProcessFailure::ProcessFailure() : msg{"execution failed"} {}
}  // namespace error

int print_error(const Error &error, const std::string &cmd) {
  std::visit([](const auto &e) { return e.msg.print(); }, error);
  usage(std::cerr, cmd);
  return 1 + error.index();
}

}  // namespace chfs
