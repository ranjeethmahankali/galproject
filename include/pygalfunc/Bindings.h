#pragma once
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/python.hpp>
#include <galfunc/Functions.h>
#include <galfunc/MeshFunctions.h>

template<size_t N, typename CppTupleType, typename... TArgs>
boost::python::tuple pythonRegisterTupleInternal(const CppTupleType cppTup, TArgs... args)
{
  static constexpr size_t tupleSize = std::tuple_size_v<CppTupleType>;
  static_assert(N <= tupleSize, "Invalid tuple accecssor");

  if constexpr (N < tupleSize) {
    return pythonRegisterTupleInternal<N + 1>(
      cppTup, std::get<N>(cppTup), args...);
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

boost::python::tuple py_loadObjFile(gal::func::store::Register filepathReg);
boost::python::tuple py_meshCentroid(gal::func::store::Register meshReg);