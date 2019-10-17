#ifndef INCLUDE_GUARD_3D6E8A94_1F8A_4440_9915_FAD6A0B1C1B1
#define INCLUDE_GUARD_3D6E8A94_1F8A_4440_9915_FAD6A0B1C1B1

#include "picojson.h"

namespace chfs {

using Json = picojson::value;

Json to_json(int n, int verbose);
Json to_json(double n, int verbose);
void json_merge(picojson::object &target, const Json &src);

template <typename C>
Json seq_to_json(const C &c, int verbose) {
  picojson::array json;
  for (const auto &e : c) {
    json.push_back(to_json(e, verbose));
  }
  return Json{std::move(json)};
}

}  // namespace chfs

#endif  // INCLUDE_GUARD_3D6E8A94_1F8A_4440_9915_FAD6A0B1C1B1
