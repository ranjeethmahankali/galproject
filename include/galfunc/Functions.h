#pragma once
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <string.h>
#include <functional>
#include <iostream>
#include <memory>

#include <boost/python.hpp>

#include <galcore/Types.h>
#include <galfunc/Converter.h>
#include <galfunc/MapMacro.h>

namespace gal {
namespace func {

// Interface.
struct Function
{
  virtual ~Function()                                 = default;
  virtual void     run()                              = 0;
  virtual void     initOutputRegisters()              = 0;
  virtual size_t   numInputs() const                  = 0;
  virtual uint64_t inputRegister(size_t index) const  = 0;
  virtual size_t   numOutputs() const                 = 0;
  virtual uint64_t outputRegister(size_t index) const = 0;
};

void unloadAllFunctions();

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

template<size_t N, typename T>
struct TupleN
{
  template<typename... Args>
  using type = typename TupleN<N - 1, T>::template type<T, Args...>;
};

template<typename T>
struct TupleN<0, T>
{
  template<typename... Args>
  using type = std::tuple<Args...>;
};

};  // namespace types

namespace store {

struct Register
{
  std::shared_ptr<void> ptr;
  std::string           typeName;
  uint64_t              id;
  const Function*       owner;
  uint32_t              typeId;
  bool                  isDirty = true;

  std::shared_ptr<Function> ownerFunc() const;
};

uint64_t allocate(const Function* fn, uint32_t typeId, const std::string& typeName);
void     free(uint64_t id);

Register& getRegister(uint64_t id);

void markDirty(uint64_t id);

void useRegister(const Function* fn, uint64_t id);

template<typename T>
void set(uint64_t id, const std::shared_ptr<T>& data)
{
  static_assert(gal::TypeInfo<T>::value, "Unknown type");
  auto& reg = getRegister(id);
  if (TypeInfo<T>::id == reg.typeId) {
    // Mark this and everything downstream as dirty.
    markDirty(reg.id);
    reg.ptr = data;
    // This register itself is not dirty.
    reg.isDirty = false;
    return;
  }
  std::cerr << "Type mismatch error: Cannot assign " << TypeInfo<T>::name() << " from "
            << reg.typeName << std::endl;
  throw std::bad_alloc();
};

template<typename T>
std::shared_ptr<T> get(uint64_t id)
{
  static_assert(TypeInfo<T>::value, "Unknown type");
  auto& reg = getRegister(id);
  if (TypeInfo<T>::id == reg.typeId || std::is_same_v<void, T>) {
    if (reg.isDirty) {
      auto fn = reg.ownerFunc();
      try {
        fn->run();
      }
      catch (std::exception e) {
        std::cerr << "Error: " << e.what() << std::endl;
      }
      reg.isDirty = false;
    }
    return std::static_pointer_cast<T>(reg.ptr);
  }
  std::cerr << "Type mismatch error: Cannot retrieve " << reg.typeName << " from "
            << TypeInfo<T>::name() << std::endl;
  throw std::bad_alloc();
};

std::shared_ptr<Function> addFunction(const std::shared_ptr<Function>& fn);

struct Lambda
{
private:
  std::vector<uint64_t> mInputs;
  std::vector<uint64_t> mOutputs;

public:
  Lambda(std::vector<uint64_t> inputs, std::vector<uint64_t> outputs);
  Lambda(const boost::python::list& inputs, const boost::python::list& outputs);

  const std::vector<uint64_t>& inputs() const;
  const std::vector<uint64_t>& outputs() const;

  template<typename T>
  void setInput(size_t i, const std::shared_ptr<T>& data) const
  {
    if (i < mInputs.size()) {
      store::set<T>(mInputs[i], data);
    }
    else {
      std::out_of_range("Lambda input index out of range.");
    }
  }

  template<typename T>
  std::shared_ptr<T> getOutput(size_t i) const
  {
    if (i < mOutputs.size()) {
      return store::get<T>(mOutputs[i]);
    }
    else {
      throw std::out_of_range("Lambda output index out of range.");
    }
  }
};

/**
 * @brief Will create dependency between captured registers of the lambda and the given
 * function.
 * @param fn The function that depends on the lambda.
 * @param lda The lambda.
 */
void useLambdaCapturedRegisters(const Function* fn, const Lambda& lda);

};  // namespace store

namespace types {
template<size_t N>
using OutputTuple = typename TupleN<N, store::Register>::template type<>;
}

template<typename... Ts>
struct TypeList
{
  static constexpr size_t NumTypes = sizeof...(Ts);
  using SharedTupleType            = std::tuple<std::shared_ptr<Ts>...>;
  using FnType                     = std::function<void(std::shared_ptr<Ts>...)>;
};

template<typename TDataList, size_t NInputs, size_t NOutputs, size_t N = 0>
struct RegisterAccessor
{
  static_assert(types::IsInstance<TypeList, TDataList>::value,
                "Inputs are not a type list.");
  using SharedTupleType = typename TDataList::SharedTupleType;
  using DType = typename std::tuple_element<N, SharedTupleType>::type::element_type;
  static constexpr size_t NumData = TDataList::NumTypes;
  static_assert(NumData == NInputs + NOutputs,
                "Number of inputs and outputs don't add up to the number of arguments");
  using NextAccessorType = RegisterAccessor<TDataList, NInputs, NOutputs, N + 1>;

public:
  static void readFromInputRegisters(const std::array<uint64_t, NumData>& ids,
                                     SharedTupleType&                     dst)
  {
    if constexpr (N < NInputs) {
      std::get<N>(dst) = store::get<DType>(ids[N]);
    }
    if constexpr (N < NInputs - 1) {
      // Recurse to the next indices.
      NextAccessorType::readFromInputRegisters(ids, dst);
    }
  };

  static void writeToOutputRegisters(const std::array<uint64_t, NumData>& ids,
                                     const SharedTupleType&               src)
  {
    if constexpr (N >= NInputs && N < NumData) {
      store::set<DType>(ids[N], std::get<N>(src));
    }
    if constexpr (N < NumData - 1) {
      // Recurse to the next indices.
      NextAccessorType::writeToOutputRegisters(ids, src);
    }
  };

  static void allocateOutputs(const Function*                fn,
                              std::array<uint64_t, NumData>& regIds,
                              SharedTupleType&               args)
  {
    static_assert(TypeInfo<DType>::value, "Unknown type");
    if constexpr (N >= NInputs && N < NumData) {
      regIds[N] =
        store::allocate(fn, TypeInfo<DType>::id, std::string(TypeInfo<DType>::name()));
      std::get<N>(args) = std::make_shared<DType>();
    }
    if constexpr (N < NumData - 1) {
      NextAccessorType::allocateOutputs(fn, regIds, args);
    }
  };
};

template<size_t NInputs, typename TArgs>
struct TFunction : public Function
{
  static_assert(types::IsInstance<TypeList, TArgs>::value,
                "Argument types are not a type list.");
  using ArgsTupleType           = typename TArgs::SharedTupleType;
  static constexpr size_t NArgs = TArgs::NumTypes;
  static_assert(NInputs <= NArgs,
                "Number of inputs is larger than the number of arguments!");
  static constexpr size_t NOutputs   = NArgs - NInputs;
  static constexpr bool   HasOutputs = NOutputs > 0;
  using FuncType                     = typename TArgs::FnType;
  using RegAccessorType              = RegisterAccessor<TArgs, NInputs, NOutputs>;

private:
  std::array<uint64_t, NArgs> mRegIds;
  ArgsTupleType               mArgs;
  FuncType                    mFunc;

public:
  TFunction(FuncType fn, const std::array<uint64_t, NInputs>& inputs)
      : mFunc(std::move(fn))
  {
    std::copy(inputs.begin(), inputs.end(), mRegIds.begin());
    for (uint64_t id : inputs) {
      store::useRegister(this, id);
    }
  };

  virtual ~TFunction()
  {
    for (size_t i = 0; i < NOutputs; i++) {
      store::free(mRegIds[i + NInputs]);
    }
  };

  size_t numInputs() const override { return NInputs; }

  uint64_t inputRegister(size_t index) const override
  {
    if (index < NInputs) {
      return mRegIds[index];
    }
    throw std::out_of_range("Input index out of range");
  }

  size_t numOutputs() const override { return NOutputs; };

  uint64_t outputRegister(size_t index) const override
  {
    if (index < NOutputs) {
      return mRegIds[index + NInputs];
    }
    throw std::out_of_range("Output index out of range");
  };

  void initOutputRegisters() override
  {
    RegAccessorType::allocateOutputs(this, mRegIds, mArgs);
  };

  void run() override
  {
    RegAccessorType::readFromInputRegisters(mRegIds, mArgs);
    std::apply(mFunc, mArgs);  // Run the function.
    RegAccessorType::writeToOutputRegisters(mRegIds, mArgs);
  };
};

template<typename TVal, typename... TArgs>
struct TVariable : public Function
{
  static constexpr bool sIsConstructible = std::is_constructible_v<TVal, TArgs...>;
  static constexpr bool sSingleArgument  = sizeof...(TArgs) == 1;

  static_assert(sIsConstructible || sSingleArgument,
                "Cannot create variable with these arguments.");

  using TFirstArg = std::tuple_element_t<0, std::tuple<TArgs...>>;

protected:
  uint64_t              mRegisterId;
  std::shared_ptr<TVal> mValuePtr;

public:
  TVariable(const TArgs&... args)
  {
    if constexpr (sIsConstructible) {
      mValuePtr = std::make_shared<TVal>(args...);
    }
    else if constexpr (sSingleArgument) {
      mValuePtr = Converter<TFirstArg, TVal>::convert(args...);
    }
    store::useRegister(this, mRegisterId);
    if constexpr (std::is_same_v<TVal, store::Lambda>) {
      store::useLambdaCapturedRegisters(this, *mValuePtr);
    }
  };

  virtual ~TVariable() { store::free(mRegisterId); }

  void initOutputRegisters() override
  {
    static_assert(TypeInfo<TVal>::value, "Unknown type");
    mRegisterId = store::allocate(this, TypeInfo<TVal>::id, TypeInfo<TVal>::name());
    store::markDirty(this->mRegisterId);
  };

  size_t numInputs() const override { return 0; }

  uint64_t inputRegister(size_t index) const override
  {
    throw std::out_of_range("Variable has no inputs.");
  }

  size_t numOutputs() const override { return 1; };

  uint64_t outputRegister(size_t index) const override
  {
    if (index == 0) {
      return mRegisterId;
    }
    throw std::out_of_range("Index out of range");
  };

  void run() override { store::set<TVal>(mRegisterId, mValuePtr); };

  void set(const TArgs&... args)
  {
    if constexpr (sIsConstructible) {
      *mValuePtr = TVal(args...);
    }
    else if constexpr (sSingleArgument) {
      Converter<TFirstArg, TVal>::assign(args..., *(this->mValuePtr));
    }
    store::markDirty(this->mRegisterId);
  };
};

namespace store {

template<typename TFunc, typename... TArgs>
std::shared_ptr<Function> makeFunction(TArgs... args)
{
  static_assert(std::is_base_of_v<Function, TFunc>, "Not a valid function type");

  auto fn = std::dynamic_pointer_cast<Function>(std::make_shared<TFunc>(args...));
  return addFunction(fn);
};

}  // namespace store

namespace types {

template<size_t NMax, size_t N = 0>
void setOutputTuple(OutputTuple<NMax>& tup, const Function& fn)
{
  if constexpr (NMax == 1) {
    tup = store::getRegister(fn.outputRegister(N));
  }
  else {
    if constexpr (N < NMax) {
      std::get<N>(tup) = store::getRegister(fn.outputRegister(N));
    }
    if constexpr (N < NMax - 1) {
      setOutputTuple<NMax, N + 1>(tup, fn);
    }
  }
};

template<size_t N>
OutputTuple<N> makeOutputTuple(const Function& fn)
{
  OutputTuple<N> tup;
  setOutputTuple<N>(tup, fn);
  return tup;
};

}  // namespace types

template<size_t NOutputs>
using PyFnOutputType =
  std::conditional_t<NOutputs == 1, store::Register, boost::python::tuple>;

template<size_t N, typename CppTupleType, typename... TArgs>
PyFnOutputType<std::tuple_size_v<CppTupleType>> pythonRegisterTupleInternal(
  const CppTupleType cppTup,
  TArgs... args)
{
  static constexpr size_t tupleSize = std::tuple_size_v<CppTupleType>;
  static_assert(tupleSize > 1,
                "If the tuple size is 1, the register should be returned directly");
  static_assert(N <= tupleSize, "Invalid tuple accecssor");
  if constexpr (N < tupleSize) {
    return pythonRegisterTupleInternal<N + 1>(cppTup, args..., std::get<N>(cppTup));
  }
  else if constexpr (N == tupleSize) {
    return boost::python::make_tuple(args...);
  }
};

template<typename... Ts>
PyFnOutputType<std::tuple_size_v<std::tuple<Ts...>>> pythonRegisterTuple(
  const std::tuple<Ts...>& cppTup)
{
  if constexpr (std::tuple_size_v<std::tuple<Ts...>> == 1) {
    return std::get<0>(cppTup);
  }
  else {
    return pythonRegisterTupleInternal<0>(cppTup);
  }
};

template<typename TVal, typename... TArgs>
PyFnOutputType<1> py_variable(const TArgs&... args)
{
  auto fn = std::dynamic_pointer_cast<Function>(
    std::make_shared<TVariable<TVal, TArgs...>>(args...));
  fn = store::addFunction(fn);
  return pythonRegisterTuple(types::makeOutputTuple<1>(*fn));
};

template<typename T>
PyFnOutputType<1> py_list(const boost::python::list& lst)
{
  return py_variable<std::vector<T>, boost::python::list>(lst);
};

}  // namespace func
}  // namespace gal

namespace std {
std::ostream& operator<<(std::ostream& ostr, const gal::func::store::Register& reg);

template<typename T>
std::ostream& operator<<(std::ostream& ostr, const std::vector<T>& vec)
{
  static constexpr char sep[] = ", ";
  static constexpr char tab   = '\t';
  ostr << "[\n";
  auto begin = vec.cbegin();
  while (begin != vec.cend()) {
    ostr << tab << *(begin++) << std::endl;
  }
  ostr << "]";
  return ostr;
};
}  // namespace std

#define GAL_CONCAT(x, y) x##y

#define _GAL_ARG_TYPE(type, name, desc) type
#define GAL_ARG_TYPE(argTuple) _GAL_ARG_TYPE argTuple

#define _GAL_ARG_NAME(type, name, desc) name
#define GAL_ARG_NAME(argTuple) _GAL_ARG_NAME argTuple

#define _GAL_EXPAND_TYPE_TUPLE(...) MAP_LIST(GAL_ARG_TYPE, __VA_ARGS__)
#define GAL_EXPAND_TYPE_TUPLE(types) _GAL_EXPAND_TYPE_TUPLE types

#define GAL_EXPAND_SHARED_ARG(argTuple) \
  std::shared_ptr<GAL_ARG_TYPE(argTuple)> GAL_ARG_NAME(argTuple)

#define GAL_EXPAND_SHARED_ARGS(...) MAP_LIST(GAL_EXPAND_SHARED_ARG, __VA_ARGS__)

#define GAL_REGISTER_ARG(typeTuple) \
  const gal::func::store::Register& GAL_ARG_NAME(typeTuple)

#define GAL_PY_REGISTER_ARG(typeTuple) gal::func::store::Register GAL_ARG_NAME(typeTuple)

#define GAL_EXPAND_REGISTER_ARGS(...) MAP_LIST(GAL_REGISTER_ARG, __VA_ARGS__)

#define GAL_EXPAND_PY_REGISTER_ARGS(...) MAP_LIST(GAL_PY_REGISTER_ARG, __VA_ARGS__)

#define GAL_EXPAND_ARG_NAMES(...) MAP_LIST(GAL_ARG_NAME, __VA_ARGS__)

#define GAL_FN_IMPL_NAME(fnName) GAL_CONCAT(fnName, _impl)

#define GAL_REGISTER_ID(argTuple) GAL_ARG_NAME(argTuple).id
#define GAL_EXPAND_REGISTER_IDS(...) MAP_LIST(GAL_REGISTER_ID, __VA_ARGS__)

#define GAL_FUNC_DECL(fnName, nInputs, nOutputs, fnDesc, inputArgs, outputArgs) \
  void GAL_FN_IMPL_NAME(fnName)(GAL_EXPAND_SHARED_ARGS inputArgs,               \
                                GAL_EXPAND_SHARED_ARGS outputArgs);             \
  gal::func::PyFnOutputType<nOutputs> py_##fnName(GAL_EXPAND_PY_REGISTER_ARGS inputArgs);

#define GAL_FUNC_DEFN(fnName, nInputs, nOutputs, fnDesc, inputArgs, outputArgs)          \
  gal::func::PyFnOutputType<nOutputs> py_##fnName(GAL_EXPAND_PY_REGISTER_ARGS inputArgs) \
  {                                                                                      \
    using FunctorType =                                                                  \
      gal::func::TFunction<nInputs,                                                      \
                           gal::func::TypeList<GAL_EXPAND_TYPE_TUPLE(inputArgs),         \
                                               GAL_EXPAND_TYPE_TUPLE(outputArgs)>>;      \
    auto fn = gal::func::store::makeFunction<FunctorType>(                               \
      GAL_FN_IMPL_NAME(fnName),                                                          \
      std::array<uint64_t, nInputs> {GAL_EXPAND_REGISTER_IDS inputArgs});                \
    return gal::func::pythonRegisterTuple(                                               \
      gal::func::types::makeOutputTuple<nOutputs>(*fn));                                 \
  };                                                                                     \
  void GAL_FN_IMPL_NAME(fnName)(GAL_EXPAND_SHARED_ARGS inputArgs,                        \
                                GAL_EXPAND_SHARED_ARGS outputArgs)

#define GAL_DEF_PY_FN(fnName) def(#fnName, py_##fnName);

// Forward declaration of the module initializer, which will be defined by boost later.
// This should be called before running scripts from within C++.
extern "C" PyObject* PyInit_pygalfunc();