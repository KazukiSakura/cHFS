#include "json.hpp"
#include <cmath>

namespace chfs {

Json to_json(int n, int) { return Json{static_cast<double>(n)}; }
Json to_json(double n, int) {
  using namespace std::string_literals;
  switch (std::fpclassify(n)) {
  case FP_INFINITE:
    return Json{"-Inf"s.substr(0.0 < n ? 1 : 0)};
  case FP_NAN:
    return Json{"NaN"};
  case FP_NORMAL:
    [[fallthrough]];
  case FP_SUBNORMAL:
    [[fallthrough]];
  case FP_ZERO:
    return Json{n};
  default:
    return Json{};
  }
}
void json_merge(picojson::object &target, const Json &src) {
  if (src.is<picojson::object>()) {
    auto &obj = src.get<picojson::object>();
    target.insert(obj.begin(), obj.end());
  }
}

}  // namespace chfs
