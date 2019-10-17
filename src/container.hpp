#ifndef INCLUDE_GUARD_1944A260_A23B_40DB_B837_5F9FF5208D78
#define INCLUDE_GUARD_1944A260_A23B_40DB_B837_5F9FF5208D78

#include <algorithm>
#include <array>
#include <iterator>
#include <queue>
#include <vector>
#include "json.hpp"

namespace chfs {

template <typename T>
struct GreaterCompare {
  constexpr bool operator()(const T &lhs, const T &rhs) const {
    return value(rhs) < value(lhs);
  }
};

template <typename T>
class Vector : private std::vector<T> {
  using Super = std::vector<T>;

 public:
  using typename Super::value_type, typename Super::size_type,
      typename Super::reference, typename Super::const_reference,
      typename Super::const_iterator;
  using Super::operator[], Super::front, Super::begin, Super::end, Super::empty,
      Super::size, Super::reserve, Super::insert, Super::erase,
      Super::push_back, Super::emplace_back, Super::pop_back;

  void insert_back(const Vector &v) { insert(end(), v.begin(), v.end()); }
  template <typename U, typename F>
  static Vector transform(const Vector<U> &seq, F f) {
    Vector result;
    result.reserve(seq.size());
    auto inserter = std::back_inserter(result);
    std::transform(seq.begin(), seq.end(), inserter, std::move(f));
    return result;
  }
};
template <typename T>
Json to_json(const Vector<T> &v, int verbose) {
  return seq_to_json(v, verbose);
}

template <typename T>
class SortedVec : private std::priority_queue<T, Vector<T>, GreaterCompare<T>> {
  using Super = std::priority_queue<T, Vector<T>, GreaterCompare<T>>;

 public:
  using Super::top, Super::empty, Super::push, Super::emplace;
};
template <typename T>
Json to_json(const SortedVec<T> &v, int verbose) {
  return to_json(v.top(), verbose);
}

template <typename T, std::size_t I>
class Array : private std::array<T, I> {
  using Super = std::array<T, I>;

 public:
  using Super::operator[], Super::begin, Super::end;
};

}  // namespace chfs

#endif  // INCLUDE_GUARD_1944A260_A23B_40DB_B837_5F9FF5208D78
