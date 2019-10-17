#ifndef INCLUDE_GUARD_16B04C00_5234_4429_AEDF_979BDA48D316
#define INCLUDE_GUARD_16B04C00_5234_4429_AEDF_979BDA48D316

#include <fstream>
#include <optional>
#include <string_view>
#include "error.hpp"

namespace picojson {
class value;
}
namespace chfs {
class Actogram;

class Parameters {
  double cycle, reso, weight;
  int verbose;
  picojson::value (*method)(const Actogram &, int);
  std::optional<std::ifstream> ifs;
  std::optional<std::ofstream> ofs;

 private:
  friend Result<Parameters> parse_args(int, char *const *);
  Parameters();
  std::optional<Error> set_cycle(std::string_view v);
  std::optional<Error> set_resolution(std::string_view v);
  std::optional<Error> set_weight(std::string_view v);
  std::optional<Error> set_verbose(std::string_view v);
  std::optional<Error> set_input(std::string_view v);
  std::optional<Error> set_output(std::string_view v);
  void set_verbose();
  void set_method();

 public:
  Result<int> run();
};

Result<Parameters> parse_args(int argc, char *const *argv);
void usage(std::ostream &os, std::string_view cmd);

}  // namespace chfs

#endif  // INCLUDE_GUARD_16B04C00_5234_4429_AEDF_979BDA48D316
