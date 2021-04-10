#pragma once
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <galfunc/Functions.h>
#include <galfunc/MeshFunctions.h>
#include <boost/python.hpp>

template<size_t N, typename CppTupleType, typename... TArgs>
boost::python::tuple pythonRegisterTupleInternal(const CppTupleType cppTup, TArgs... args)
{
  static constexpr size_t tupleSize = std::tuple_size_v<CppTupleType>;
  static_assert(N <= tupleSize, "Invalid tuple accecssor");

  if constexpr (N < tupleSize) {
    return pythonRegisterTupleInternal<N + 1>(cppTup, args..., std::get<N>(cppTup));
  }
  else if constexpr (N == tupleSize) {
    return boost::python::make_tuple(args...);
  }
};

template<typename... Ts>
boost::python::tuple pythonRegisterTuple(const std::tuple<Ts...>& cppTup)
{
  return pythonRegisterTupleInternal<0>(cppTup);
};

template<typename T>
boost::python::tuple py_constant(const T& value)
{
  return pythonRegisterTuple(gal::func::constant<T>(value));
};

template<typename T>
T py_readRegister(gal::func::store::Register reg)
{
  return *gal::func::store::get<T>(reg.id);
};

boost::python::tuple py_loadObjFile(gal::func::store::Register filepathReg);
boost::python::tuple py_meshCentroid(gal::func::store::Register meshReg);