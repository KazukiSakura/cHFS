#include "params.hpp"

int main(int argc, char *argv[]) {
  if (auto params = chfs::parse_args(argc, argv)) {
    return chfs::print_error(params->run(), argv[0]);
  } else {
    return chfs::print_error(params, argv[0]);
  }
}
