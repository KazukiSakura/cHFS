#ifndef INCLUDE_GUARD_200335F6_8FC7_4378_A356_58FD616D3C98
#define INCLUDE_GUARD_200335F6_8FC7_4378_A356_58FD616D3C98

#include "container.hpp"
#include "json.hpp"

namespace chfs {
class Code;

class Record : private std::array<int, 3> {
 public:
  Record(int v, int b, int e);
  int value() const;
  int begin() const;
  int end() const;
  Code binarize(int thres) const;
  int count(int thres) const;
};

class Code : private Record {
 public:
  Code(bool z, int b, int e);
  using Record::begin, Record::end;
  bool iszero() const;
  int code() const;
};
Json to_json(const Code &rec, int verbose);
using CodeSeq = Vector<Code>;

class Actogram;
class ActogramSlice {
  const Actogram &acto;
  int begin, end;

 public:
  ActogramSlice(const Actogram &a, int b, int e);
  int size() const;
  double cycle() const;
  double resolution() const;
  double weight() const;
  int width() const;
  int height() const;
  CodeSeq sequence() const;
  Vector<ActogramSlice> slices(int count) const;
};
class Actogram {
  Vector<int> acto;
  double cycle, reso, weight;
  friend class ActogramSlice;

 public:
  Actogram(const Vector<int> &csv, double c, double r, double w);
  Code at(int i) const;
  ActogramSlice slice() const;
};

}  // namespace chfs

#endif  // INCLUDE_GUARD_200335F6_8FC7_4378_A356_58FD616D3C98
