#ifndef INCLUDE_GUARD_4F86D9DF_11EE_49F5_9555_137C0F9A507D
#define INCLUDE_GUARD_4F86D9DF_11EE_49F5_9555_137C0F9A507D

#include "container.hpp"
#include "json.hpp"

namespace chfs {
class ActogramSlice;

using OnsetTimes = Vector<double>;
using Thresholds = Array<int, 3>;
using ScoreElem = Array<double, 5>;
using Score = SortedVec<ScoreElem>;

double value(const ScoreElem &elem);
double value(const Score &score);

Json to_json(const Thresholds &thres, int verbose);
Json to_json(const ScoreElem &elem, int verbose);
Json to_json(const Score &score, int verbose);

Score calc_score(const ActogramSlice &acto, const OnsetTimes &onsets);

}  // namespace chfs

#endif  // INCLUDE_GUARD_4F86D9DF_11EE_49F5_9555_137C0F9A507D
