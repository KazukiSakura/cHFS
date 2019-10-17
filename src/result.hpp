#ifndef INCLUDE_GUARD_AB644267_F64D_4246_AF44_EA40B259E804
#define INCLUDE_GUARD_AB644267_F64D_4246_AF44_EA40B259E804

#include <functional>
#include <utility>
#include <variant>

namespace result {
namespace detail {
namespace tag {
struct Ok {};
struct Err {};
}  // namespace tag

namespace holder {
template <typename Tag, typename T>
class Holder {
  T t;

 public:
  template <typename U>
  explicit Holder(U &&u) : t{std::forward<U>(u)} {}
  T get() && { return std::move(t); }
  T &get() & = delete;
};
template <typename Tag, typename T>
class Holder<Tag, T &> {
  T &t;

 public:
  template <typename U>
  explicit Holder(U &u) : t{u} {}
  T &get() && { return t; }
  T &get() & = delete;
};

template <typename T>
using Ok = Holder<tag::Ok, T>;
template <typename E>
using Err = Holder<tag::Err, E>;
}  // namespace holder

namespace storage {
template <typename Tag, typename T>
class Storage {
  T t;

 public:
  template <typename U>
  explicit Storage(U &&u) : t{std::forward<U>(u)} {}
  T get() && { return std::move(t); }
  T &get() & { return t; }
  const T &get() const & { return t; }
};
template <typename Tag, typename T>
class Storage<Tag, T &> : private std::reference_wrapper<T> {
  using Super = std::reference_wrapper<T>;

 public:
  using Super::Super, Super::get;
};

template <typename T>
using Ok = Storage<tag::Ok, T>;
template <typename E>
using Err = Storage<tag::Err, E>;
}  // namespace storage
}  // namespace detail

template <typename T, typename E>
class Result {
  using Ok = detail::storage::Ok<T>;
  using Err = detail::storage::Err<E>;
  std::variant<Ok, Err> s;

 public:
  template <typename U>
  Result(detail::holder::Ok<U> &&u) : s{Ok{std::move(u).get()}} {}
  template <typename U>
  Result(detail::holder::Err<U> &&u) : s{Err{std::move(u).get()}} {}
  explicit operator bool() const { return s.index() == 0; }
  T operator*() && { return value(); }
  T &operator*() & { return value(); }
  const T &operator*() const & { return value(); }
  auto operator-> () { return &operator*(); }
  auto operator-> () const { return &operator*(); }
  T value() && { return std::get<0>(std::move(s)).get(); }
  T &value() & { return std::get<0>(s).get(); }
  const T &value() const & { return std::get<0>(s).get(); }
  E error() && { return std::get<1>(std::move(s)).get(); }
  E &error() & { return std::get<1>(s).get(); }
  const E &error() const & { return std::get<1>(s).get(); }
};

namespace detail {
template <typename>
struct ResultTrait : std::false_type {};
template <typename T, typename E>
struct ResultTrait<Result<T, E>> : std::true_type {
  using Ok = holder::Ok<T>;
  using Err = holder::Err<E>;
};
}  // namespace detail

template <typename T>
auto ok(T &&x) {
  using Trait = detail::ResultTrait<std::remove_reference_t<T>>;
  if constexpr (Trait::value) {
    return typename Trait::Ok{std::forward<T>(x).value()};
  } else {
    return detail::holder::Ok<T>{std::forward<T>(x)};
  }
}
template <typename E>
auto err(E &&x) {
  using Trait = detail::ResultTrait<std::remove_reference_t<E>>;
  if constexpr (Trait::value) {
    return typename Trait::Err{std::forward<E>(x).error()};
  } else {
    return detail::holder::Err<E>{std::forward<E>(x)};
  }
}
}  // namespace result

#endif  // INCLUDE_GUARD_AB644267_F64D_4246_AF44_EA40B259E804
