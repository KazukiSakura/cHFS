#ifndef INCLUDE_GUARD_7BE5D489_8A5C_4142_AB52_1A445B23E86E
#define INCLUDE_GUARD_7BE5D489_8A5C_4142_AB52_1A445B23E86E

#include <tuple>
#include "actogram.hpp"
#include "container.hpp"
#include "json.hpp"
#include "score.hpp"

namespace chfs {

using HfsSeqs = std::array<CodeSeq, 3>;
using HfsElem = std::tuple<Score, OnsetTimes, Thresholds, HfsSeqs>;
using Hfs = SortedVec<HfsElem>;
using HfsParts = Vector<Hfs>;
using ChfsElem = std::tuple<Score, OnsetTimes, HfsParts>;
using Chfs = SortedVec<ChfsElem>;

double value(const HfsElem &elem);
double value(const ChfsElem &elem);

Json to_json(const HfsElem &elem, int verbose);
Json to_json(const ChfsElem &elem, int verbose);

Json do_hfs(const Actogram &acto, int verbose);
Json do_chfs(const Actogram &acto, int verbose);

}  // namespace chfs

#endif  // INCLUDE_GUARD_7BE5D489_8A5C_4142_AB52_1A445B23E86E
