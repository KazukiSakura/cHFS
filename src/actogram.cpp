#include "actogram.hpp"
#include "calc.hpp"

namespace chfs {

Record::Record(int v, int b, int e) : std::array<int, 3>{v, b, e} {}
int Record::value() const { return (*this)[0]; }
int Record::begin() const { return (*this)[1]; }
int Record::end() const { return (*this)[2]; }
Code Record::binarize(int thres) const {
  return {thres <= value(), begin(), end()};
}
int Record::count(int thres) const {
  return thres <= value() ? calc::ceil(value(), thres) + 1 : 1;
}

Code::Code(bool z, int b, int e) : Record{z, b, e} {}
bool Code::iszero() const { return value(); }
int Code::code() const { return value() ? 0 : 1; }
Json to_json(const Code &rec, int verbose) {
  if (4 < verbose) {
    picojson::object json;
    json["code"] = to_json(rec.code(), verbose);
    json["begin"] = to_json(rec.begin(), verbose);
    json["end"] = to_json(rec.end(), verbose);
    return Json{std::move(json)};
  } else {
    return to_json(rec.code(), verbose);
  }
}

ActogramSlice::ActogramSlice(const Actogram &a, int b, int e)
    : acto{a}, begin{b}, end{e} {}
int ActogramSlice::size() const { return end - begin; }
double ActogramSlice::cycle() const { return acto.cycle; }
double ActogramSlice::resolution() const { return acto.reso; }
double ActogramSlice::weight() const { return acto.weight; }
int ActogramSlice::width() const { return calc::round(acto.cycle, acto.reso); }
int ActogramSlice::height() const { return calc::ceil(size(), width()); }
CodeSeq ActogramSlice::sequence() const {
  CodeSeq seq;
  seq.reserve(size());
  for (auto i = begin; i < end; ++i) {
    seq.push_back(acto.at(i));
  }
  return seq;
}
Vector<ActogramSlice> ActogramSlice::slices(int count) const {
  Vector<ActogramSlice> slices;
  slices.reserve(count);
  int l = size();
  for (int i = 0, j = 1; i < count; ++i, ++j) {
    auto b = calc::round(i * l, count);
    auto e = calc::round(j * l, count);
    slices.emplace_back(acto, b, e);
  }
  return slices;
}
Actogram::Actogram(const Vector<int> &csv, double c, double r, double w)
    : acto{csv}, cycle{c}, reso{r}, weight{w} {}
Code Actogram::at(int i) const {
  return Code{static_cast<bool>(acto[i]), i, i};
}
auto Actogram::slice() const -> ActogramSlice {
  int s = acto.size();
  return ActogramSlice{*this, 0, s};
}

}  // namespace chfs
