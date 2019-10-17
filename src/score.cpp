#include "score.hpp"
#include <cassert>
#include <cmath>
#include <numeric>
#include "actogram.hpp"
#include "calc.hpp"

namespace chfs {
namespace {
enum ScoreEnum { ALPHA, BETA, MSE, PEN, DELTA };
using TimeSeq = Vector<double>;

std::tuple<double, double> alpha_beta(const TimeSeq &xs, const TimeSeq &ys) {
  auto sum = [](const TimeSeq &seq) {
    return std::accumulate(begin(seq), end(seq), 0.0);
  };
  auto dot = [](const TimeSeq &lhs, const TimeSeq &rhs) {
    return std::inner_product(begin(lhs), end(lhs), begin(rhs), 0.0);
  };
  auto n = xs.size();
  auto sum_x = sum(xs);
  auto sum_y = sum(ys);
  auto xy = n * dot(xs, ys) - sum_x * sum_y;
  auto yy = n * dot(ys, ys) - sum_y * sum_y;
  auto alpha = xy / yy;
  auto beta = (sum_x - alpha * sum_y) / n;
  return {alpha, beta};
}
double mse(double alpha, double beta, const TimeSeq &xs, const TimeSeq &ys) {
  auto square = [](double lhs, double rhs) { return lhs + rhs * rhs; };
  auto line = [alpha, beta](double x, double y) {
    return x - alpha * y - beta;
  };
  auto sse =
      std::inner_product(begin(xs), end(xs), begin(ys), 0.0, square, line);
  auto n = xs.size();
  return sse / (n - 2.0);
}
double penalty(TimeSeq uniq, int height) {
  std::sort(begin(uniq), end(uniq));
  auto last = std::unique(begin(uniq), end(uniq));
  auto num_of_onsets = uniq.size();
  auto num_of_rows = height;
  auto num_of_with = std::distance(begin(uniq), last);
  auto num_of_without = num_of_rows - num_of_with;
  auto num_of_dup = num_of_onsets - num_of_with;
  return num_of_without + num_of_dup;
}
ScoreElem linear_regression(const OnsetTimes &onsets, double delta,
                            double cycle, double weight, int height) {
  auto rem = [delta, cycle](double z) {
    return calc::remainder(z + delta, cycle);
  };
  auto quo = [delta, cycle](double z) {
    return calc::quotient(z + delta, cycle);
  };
  auto xs = TimeSeq::transform(onsets, std::move(rem));
  auto ys = TimeSeq::transform(onsets, std::move(quo));
  ScoreElem elem;
  std::tie(elem[ALPHA], elem[BETA]) = alpha_beta(xs, ys);
  elem[MSE] = mse(elem[ALPHA], elem[BETA], xs, ys);
  elem[PEN] = weight * penalty(ys, height);
  elem[DELTA] = delta;
  return elem;
}
bool isfinite(const ScoreElem &elem) {
  for (const auto &e : elem) {
    if (!std::isfinite(e)) {
      return false;
    }
  }
  return true;
}
}  // namespace

double value(const ScoreElem &elem) { return elem[MSE] + elem[PEN]; }
double value(const Score &score) { return value(score.top()); }

Json to_json(const Thresholds &thres, int verbose) {
  return seq_to_json(thres, verbose);
}
Json to_json(const ScoreElem &elem, int verbose) {
  picojson::object json;
  json["MSE"] = to_json(elem[MSE], verbose);
  json["penalty"] = to_json(elem[PEN], verbose);
  if (2 < verbose) {
    json["alpha"] = to_json(elem[ALPHA], verbose);
    json["beta"] = to_json(elem[BETA], verbose);
    json["delta"] = to_json(elem[DELTA], verbose);
  }
  return Json{std::move(json)};
}
Json to_json(const Score &score, int verbose) {
  picojson::object json;
  auto &elem = score.top();
  json["score"] = to_json(value(elem), verbose);
  if (1 < verbose) {
    json["regression"] = to_json(elem, verbose);
  }
  return Json{std::move(json)};
}

Score calc_score(const ActogramSlice &acto, const OnsetTimes &onsets) {
  assert(2 < onsets.size());
  Score score;
  auto head = onsets.front();
  auto cycle = acto.cycle();
  auto weight = acto.weight();
  auto height = acto.height();
  for (const auto &e : onsets) {
    auto left = calc::remainder(e, cycle);
    auto delta = (head < left ? cycle : 0) - left;
    auto elem = linear_regression(onsets, delta, cycle, weight, height);
    if (isfinite(elem)) {
      score.push(elem);
    }
  }
  return score;
}

}  // namespace chfs
