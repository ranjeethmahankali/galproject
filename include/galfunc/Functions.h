#pragma once

#include <functional>
#include <tuple>

namespace gal {
namespace func {

/**
 * @brief Picks atype from the given list of types
 * @tparam N The index of the type to be picked.
 * @tparam Ts The list of types.
 */
template<size_t N, typename... Ts>
struct TypePicker
{
};

/**
 * @brief Template specialization for picking a single type.
 * This is the terminating case for the recursive template.
 * @tparam N The type index, must be zero.
 * @tparam T Type.
 */
template<size_t N, typename T>
struct TypePicker<N, T>
{
  static_assert(N == 0, "Type index out of range");
  using type = T;
};

/**
 * @brief Template specialization for more than one types.
 * @tparam N The index of the type to be picked.
 * @tparam TFirst First type.
 * @tparam TRest The remaining types.
 */
template<size_t N, typename TFirst, typename... TRest>
struct TypePicker<N, TFirst, TRest...>
{
  using type =
    std::conditional_t<N == 0, TFirst, typename TypePicker<N - 1, TRest...>::type>;
};

template<typename... Ts>
struct TypeSet
{
  template<size_t N>
  using type = typename TypePicker<N, Ts...>::type;

  using TupleType = std::tuple<Ts...>;

  template<typename RetType>
  using FuncType = std::function<RetType(const Ts&...)>;

  static constexpr size_t NumTypes = sizeof...(Ts);
};

template<typename TRet, typename... TArgs>
struct Functor
{
public:
  using FuncType = std::function<TRet(const TArgs&...)>;

private:
  FuncType mFunc;

public:
  Functor(const FuncType& func)
      : mFunc(func) {};

  TRet operator()(const TArgs&... args)
  {
    return mFunc(args...);
  };
};

}  // namespace func
}  // namespace gal