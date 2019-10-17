#include "hfs.hpp"
#include "calc.hpp"

namespace chfs {
namespace {
using RecurrenceSeq = Vector<Record>;
Vector<int> candidates(const RecurrenceSeq &seq) {
  auto f = [](const auto &rec) { return rec.value(); };
  auto v = Vector<int>::transform(seq, std::move(f));
  std::sort(begin(v), end(v));
  v.erase(std::unique(begin(v), end(v)), end(v));
  v.erase(std::remove(begin(v), end(v), 0), end(v));
  return v;
}
RecurrenceSeq extract(const CodeSeq &seq) {
  int size = seq.size();
  RecurrenceSeq result;
  result.emplace_back(-1, -1, -1);
  for (int i = 0; i < size; ++i) {
    if (seq[i].iszero()) {
      result.emplace_back(i, seq[i].begin(), seq[i].end());
    }
  }
  result.emplace_back(size, -1, -1);
  return result;
}
RecurrenceSeq recurrence(const CodeSeq &seq) {
  RecurrenceSeq result;
  auto zeros = extract(seq);
  if (auto prev = zeros.begin(); prev != zeros.end()) {
    for (auto next = std::next(prev); next != zeros.end(); ++prev, ++next) {
      result.emplace_back(next->value() - prev->value() - 1, prev->end(),
                          next->begin());
    }
  }
  return result;
}
CodeSeq filter(const RecurrenceSeq &seq, int thres, bool duplicate) {
  CodeSeq result;
  for (const auto &c : seq) {
    auto count = duplicate ? c.count(thres) : 1;
    for (auto i = 0; i < count; ++i) {
      result.push_back(c.binarize(thres));
    }
  }
  return result;
}
OnsetTimes detect(const CodeSeq &seq, double reso) {
  OnsetTimes result;
  for (const auto &c : seq) {
    if (c.iszero() && 0 <= c.end()) {
      result.push_back(reso * c.end());
    }
  }
  return result;
}
Hfs calc_hfs(const ActogramSlice &acto) {
  Hfs hfs;
  Thresholds thres;
  auto reso = acto.resolution();
  auto seq = acto.sequence();
  auto time_series = recurrence(seq);
  for (const auto &delta : candidates(time_series)) {
    thres[0] = delta;
    auto czero = filter(time_series, delta, true);
    auto sr_seq = recurrence(czero);
    for (const auto &h_sr : candidates(sr_seq)) {
      thres[1] = h_sr;
      auto cstar = filter(sr_seq, h_sr, false);
      auto mp_seq = recurrence(cstar);
      for (const auto &h_mp : candidates(mp_seq)) {
        thres[2] = h_mp;
        auto cat = filter(mp_seq, h_mp, false);
        auto onsets = detect(cat, reso);
        if (2 < onsets.size()) {
          auto score = calc_score(acto, onsets);
          hfs.emplace(std::move(score), std::move(onsets), thres,
                      HfsSeqs{czero, cstar, cat});
        }
      }
    }
  }
  return hfs;
}

std::optional<ChfsElem> collect(const ActogramSlice &acto, int i) {
  OnsetTimes onsets;
  HfsParts parts;
  for (const auto &slice : acto.slices(i)) {
    auto hfs = calc_hfs(slice);
    if (hfs.empty()) {
      return std::nullopt;
    }
    onsets.insert_back(std::get<1>(hfs.top()));
    parts.push_back(std::move(hfs));
  }
  auto score = calc_score(acto, onsets);
  return ChfsElem{std::move(score), std::move(onsets), std::move(parts)};
}
Chfs calc_chfs(const ActogramSlice &acto) {
  Chfs chfs;
  constexpr int min_regression = 3;
  const int max_parts = calc::floor(acto.resolution() * acto.size(),
                                    acto.cycle() * min_regression);
  for (int i = 1; i <= max_parts; ++i) {
    if (auto e = collect(acto, i)) {
      chfs.push(std::move(*e));
    }
  }
  return chfs;
}
}  // namespace

double value(const HfsElem &elem) { return value(std::get<0>(elem)); }
double value(const ChfsElem &elem) { return value(std::get<0>(elem)); }

Json to_json(const HfsElem &elem, int verbose) {
  if (0 == verbose) {
    return to_json(std::get<1>(elem), verbose);
  } else {
    picojson::object json;
    json_merge(json, to_json(std::get<0>(elem), verbose));
    json["onsets"] = to_json(std::get<1>(elem), verbose);
    json["threshold"] = to_json(std::get<2>(elem), verbose);
    if (3 < verbose) {
      auto &seqs = std::get<3>(elem);
      json["C0"] = to_json(seqs[0], verbose);
      json["C*"] = to_json(seqs[1], verbose);
      json["C@"] = to_json(seqs[2], verbose);
    }
    return Json{std::move(json)};
  }
}
Json to_json(const ChfsElem &elem, int verbose) {
  if (0 == verbose) {
    return to_json(std::get<1>(elem), verbose);
  } else {
    picojson::object json;
    json_merge(json, to_json(std::get<0>(elem), verbose));
    json["onsets"] = to_json(std::get<1>(elem), verbose);
    if (1 < verbose) {
      json["HFS"] = to_json(std::get<2>(elem), verbose);
    }
    return Json{std::move(json)};
  }
}

Json do_hfs(const Actogram &acto, int verbose) {
  auto hfs = calc_hfs(acto.slice());
  if (hfs.empty()) {
    return Json{};
  }
  return to_json(hfs, verbose);
}
Json do_chfs(const Actogram &acto, int verbose) {
  auto chfs = calc_chfs(acto.slice());
  if (chfs.empty()) {
    return Json{};
  }
  return to_json(chfs, verbose);
}

}  // namespace chfs
