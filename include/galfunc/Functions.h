#pragma once

#include <string.h>
#include <functional>

namespace gal {
namespace func {

namespace types {

// Can be used to check at compile time if a type is a template instantiation.
template<template<typename...> typename Tem, typename T>
struct IsInstance : public std::false_type
{
};

// Template specialization that does the magic.
template<template<typename...> typename Tem, typename... Ts>
struct IsInstance<Tem, Tem<Ts...>> : public std::true_type
{
};

template<typename T>
struct TypeInfo : public std::false_type
{
  static constexpr uint32_t id = 0U;
};

};  // namespace types

namespace store {

uint64_t              allocate();
void                  free(uint64_t id);
void                  set(uint64_t id, const std::shared_ptr<void>& data);
std::shared_ptr<void> get(uint64_t id);

};  // namespace store

template<typename... Ts>
struct TypeList
{
  using SharedTupleType            = std::tuple<std::shared_ptr<Ts>...>;
  static constexpr size_t numTypes = sizeof...(Ts);
};

template<typename TInputs, typename TOutputs>
struct TFunction
{
  static_assert(IsInstance<TypeList, TInputs>::value, "Inputs are not a type list.");
  static_assert(IsInstance<TypeList, TOutputs>::value, "Outputs are not a type list.");

  using InputsType  = TInputs::SharedTupleType;
  using OutputsType = TOutputs::SharedTupleType;

  void run(const std::array<uint64_t, TInputs::numTypes>& ids);

  template<uin64_t... Ids>
  void run(Ids... ids)
  {
    static_assert(sizeof...(Ids) == TInputs::numTypes, "Incorrect number of types");
    run(std::array<uint64_t, TInputs::numTypes> {ids...});
  };

private:
  OutputsType mOutputs;
};

}  // namespace func
}  // namespace gal
