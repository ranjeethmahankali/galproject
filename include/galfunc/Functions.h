#pragma once

#include <string.h>
#include <functional>
#include <iostream>
#include <memory>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/python.hpp>
#include <boost/python/scope.hpp>
#include <boost/python/tuple.hpp>

#include <galcore/Util.h>
#include <galfunc/Converter.h>
#include <galfunc/Data.h>
#include <galfunc/MapMacro.h>

namespace gal {
namespace func {

/**
 * @brief Base class for all functions.
 */
struct Function
{
  // Virtual destructor because of polymorphism.
  virtual ~Function() = default;

  /**
   * @brief Ensures the output data is up to date.
   */
  virtual void update() const = 0;

  virtual void addSubscriber(std::atomic_bool& dirtyFlag) const = 0;
};

namespace store {

/**
 * @brief Adds the function instance to the list of managed functions. This is necessary
 * because all function instances must be tracked and managed.
 *
 * @param fn
 */
std::shared_ptr<Function> addFunction(const std::shared_ptr<Function>& fn);

/**
 * @brief Unloads all loaded function instances.
 */
void unloadAllFunctions();

/**
 * @brief Subscribes a boolean flag to the function. When the function is dirty (not upto
 * date), the subscribed boolean flag will be set to true. Its upto the owning function to
 * recompute itself and set the flag back to true.
 *
 * @param fn The function to subscribe to.
 * @param dirtyFlag The flag to subscribe.
 */
void addSubscriber(const Function* fn, std::atomic_bool& dirtyFlag);

/**
 * @brief Lets all subscribers of this function know that this function is dirty.
 *
 * @param fn the function to be marked dirty.
 */
void markDirty(const Function* fn);

};  // namespace store

/**
 * @brief Wrapper that points to readonly data owned by a function. These are passed down
 * to python to construct the function graph.
 *
 * @tparam T
 */
template<typename T>
struct Register
{
  static_assert(!data::IsReadView<T>::value && !data::IsWriteView<T>::value,
                "Can't have registers of views.");
  const Function*      mOwner = nullptr;
  const data::Tree<T>* mData  = nullptr;

  Register() = default;

  Register(const Function* fn, const data::Tree<T>* dref)
      : mOwner(fn)
      , mData(dref)
  {}

  const data::Tree<T>& read() const
  {
    mOwner->update();
    return *mData;
  }
};

/**
 * Helper templates to wrangle various types at compile time.
 */

template<typename T>
struct ImplFnArgType
{
  using Type =
    std::conditional_t<(data::IsReadView<T>::value || data::IsWriteView<T>::value),
                       std::remove_const_t<T>,
                       T&>;
};

template<typename T>
struct ArgTree
{
  using Type = data::Tree<typename data::UnwrapView<T>::Type>;
};

template<typename T>
struct ArgTree<const T>
{
  using Type = const typename ArgTree<T>::Type;
};

template<typename T>
struct ArgTree<data::Tree<T>>
{
  using Type = data::Tree<T>;
};

template<typename T>
using ArgTreeT = typename ArgTree<T>::Type;

template<typename T>
struct ArgRegister
{
  using Type = Register<std::remove_const_t<typename data::UnwrapView<T>::Type>>;
};

template<typename T>
struct ArgRegister<const T>
{
  // Register always point to const trees, this type doesn't need to be const.
  using Type = typename ArgRegister<T>::Type;
};

template<typename T>
struct ArgRegister<data::Tree<T>>
{
  using Type = Register<std::remove_const_t<T>>;
};

template<typename T>
using ArgRegisterT = typename ArgRegister<T>::Type;

/**
 * @brief Helper template to process the variadic argument type list.
 *
 * @tparam TArgs Argument types.
 */
template<typename... TArgs>
struct TypeList
{
  /**
   * @brief Trait that counts the number of leading const types in the parameter pack.
   */
  template<typename... Ts>
  struct NumConstTypes;

  // Partial specialization of the template for a single type.
  template<typename T>
  struct NumConstTypes<T>
  {
    static constexpr size_t value = std::is_const_v<T> ? 1 : 0;
  };

  // Partial specialization of the template for more than one type.
  template<typename T, typename... Ts>
  struct NumConstTypes<T, Ts...>
  {
    static constexpr size_t value =
      std::is_const_v<T> ? (1 + NumConstTypes<Ts...>::value) : 0;
  };

  /**
   * @brief For the argument type list to be valid, a const type
   * should never follow a non-const type. Because outputs must follow inputs. This
   * template checks for that.
   *
   * @tparam Ts
   */
  template<typename... Ts>
  struct ValidConstOrder;

  // Partial specialization for one type.
  template<typename T>
  struct ValidConstOrder<T> : std::true_type
  {
    static constexpr bool IsFirstConst = std::is_const_v<T>;
  };

  // Partial specialization for more than one type.
  template<typename T, typename... Ts>
  struct ValidConstOrder<T, Ts...>
  {
    static constexpr bool IsFirstConst = std::is_const_v<T>;
    static constexpr bool value =
      (IsFirstConst || !ValidConstOrder<Ts...>::IsFirstConst) &&
      ValidConstOrder<Ts...>::value;
  };

  template<size_t NBegin, size_t NEnd, typename... Ts>
  struct SubTuples
  {
    static_assert(NEnd <= sizeof...(Ts));
    // static_assert(NBegin < NEnd);

    using T          = std::tuple_element_t<NBegin, std::tuple<Ts...>>;
    using UnwrappedT = typename data::UnwrapView<T>::Type;
    static_assert(!data::IsReadView<UnwrappedT>::value &&
                  !data::IsWriteView<UnwrappedT>::value);
    using RegisterT = ArgRegisterT<T>;
    using TreeT     = ArgTreeT<T>;

    template<typename H, typename... Us>
    using AppendedTupleT = std::conditional_t<
      IsInstance<Register, H>::value,
      /*appended reg tuple*/
      typename SubTuples<NBegin + 1, NEnd, Ts...>::template RegTupleType<Us..., H>,
      std::conditional_t<
        IsInstance<data::Tree, H>::value,
        /*appended tree tuple*/
        typename SubTuples<NBegin + 1, NEnd, Ts...>::template TreeTupleType<Us..., H>,
        /*appended tuple*/
        typename SubTuples<NBegin + 1, NEnd, Ts...>::template TupleType<Us..., T>>>;

    // sub tuple types.
    template<typename... Us>
    using TupleType = typename std::
      conditional_t<(NBegin < NEnd), AppendedTupleT<T, Us...>, std::tuple<Us...>>;

    template<typename... Us>
    using TreeTupleType = typename std::
      conditional_t<(NBegin < NEnd), AppendedTupleT<TreeT, Us...>, std::tuple<Us...>>;

    template<typename... Us>
    using RegTupleType = typename std::
      conditional_t<(NBegin < NEnd), AppendedTupleT<RegisterT, Us...>, std::tuple<Us...>>;
  };

  template<size_t NEnd, typename... Ts>
  struct SubTuples<NEnd, NEnd, Ts...>
  {
    template<typename... Us>
    using TupleType = std::tuple<Us...>;

    template<typename... Us>
    using TreeTupleType = std::tuple<Us...>;

    template<typename... Us>
    using RegTupleType = std::tuple<Us...>;
  };

  static_assert(ValidConstOrder<TArgs...>::value);
  static constexpr size_t NumTypes  = sizeof...(TArgs);
  static constexpr size_t NumInputs = NumConstTypes<TArgs...>::value;
  using TupleT                      = std::tuple<TArgs...>;
  using ArgTupleType                = std::tuple<typename ImplFnArgType<TArgs>::Type...>;
  using PtrTupleType                = std::tuple<TArgs*...>;
  using OutputTupleType =
    typename SubTuples<NumInputs, NumTypes, TArgs...>::template TreeTupleType<>;
  using InputRegTupleType =
    typename SubTuples<0, NumInputs, TArgs...>::template RegTupleType<>;
  using ImplFnType = std::function<void(typename ImplFnArgType<TArgs>::Type...)>;

  template<size_t N>
  using Type = typename std::tuple_element<N, std::tuple<TArgs...>>::type;

  using ArgTreeRefTupleT = std::tuple<typename ArgTree<TArgs>::Type&...>;
};

template<typename TArgList, size_t N>
static ArgTreeT<typename TArgList::template Type<N>>& getTreeRef(
  const typename TArgList::InputRegTupleType& inputs,
  typename TArgList::OutputTupleType&         outputs)
{
  using DType = typename TArgList::template Type<N>;
  static_assert(TypeInfo<DType>::value, "Unknown type");
  static_assert(TArgList::NumTypes > N);
  if constexpr (N < TArgList::NumInputs) {
    return *(std::get<N>(inputs).mData);
  }
  else {
    return std::get<N - TArgList::NumInputs>(outputs);
  }
}

// For internal use only from the other function.
template<typename TArgList, size_t... Is>
typename TArgList::ArgTreeRefTupleT makeArgTreeRefTupleInternal(
  const typename TArgList::InputRegTupleType& inputs,
  typename TArgList::OutputTupleType&         outputs,
  std::index_sequence<Is...>)
{
  return std::tie(getTreeRef<TArgList, Is>(inputs, outputs)...);
}

template<typename TArgList>
typename TArgList::ArgTreeRefTupleT makeArgTreeRefTuple(
  const typename TArgList::InputRegTupleType& inputs,
  typename TArgList::OutputTupleType&         outputs)
{
  return makeArgTreeRefTupleInternal<TArgList>(
    inputs, outputs, std::make_index_sequence<TArgList::NumTypes> {});
}

// For internal use only from the other function.
template<typename OutputTupleT, size_t... Is>
boost::python::tuple pythonOutputTupleInternal(const Function*     fn,
                                               const OutputTupleT& src,
                                               std::index_sequence<Is...>)
{
  return boost::python::make_tuple(
    ArgRegisterT<typename std::tuple_element_t<Is, OutputTupleT>::Type>(
      fn, &(std::get<Is>(src)))...);
}

/**
 * @brief Wraps the output tuple of the function in registers and returns a python tuple
 * of registers.
 *
 * @tparam OutputTupleT The output tuple type.
 * @param fn The function that owns the outputs.
 * @param src The output tuple. It is the source of the wrapped register tuple.
 * @return boost::python::tuple
 */
template<typename OutputTupleT>
boost::python::tuple pythonOutputTuple(const Function* fn, const OutputTupleT& src)
{
  return pythonOutputTupleInternal(
    fn, src, std::make_index_sequence<std::tuple_size_v<OutputTupleT>> {});
}

/**
 * @brief Template for all non-variable functions.
 * @tparam NInputs Number of inputs.
 * @tparam TArgs All arguments. Inputs followed by outputs.
 */
template<typename... TArgs>
struct TFunction : public Function
{
  using TArgList                  = TypeList<TArgs...>;
  static constexpr size_t NInputs = TArgList::NumInputs;
  using ArgTupleT                 = typename TArgList::ArgTupleType;
  using ArgTreeRefTupleT          = typename TArgList::ArgTreeRefTupleT;
  using InputRegTupleT            = typename TArgList::InputRegTupleType;
  using OutputTupleT              = typename TArgList::OutputTupleType;
  static constexpr size_t NArgs   = TArgList::NumTypes;
  static_assert(NInputs <= NArgs,
                "Number of inputs is larger than the number of arguments!");
  static constexpr bool   HasInputs  = NInputs > 0;
  static constexpr size_t NOutputs   = NArgs - NInputs;
  static constexpr bool   HasOutputs = NOutputs > 0;
  using ImplFuncType                 = typename TArgList::ImplFnType;
  using PyOutputType =
    std::conditional_t<(NOutputs > 1),
                       boost::python::tuple,
                       ArgRegisterT<typename TArgList::template Type<NInputs>>>;

protected:
  /* Some fields are marked mutable because the function is considered changed only if the
   * inputs are changed. Running the function, which changes the output, or the status of
   * the dirty-flag, is not considered changing.
   */
  ImplFuncType                                                            mFunc;
  mutable OutputTupleT                                                    mOutputs;
  InputRegTupleT                                                          mInputs;
  mutable data::repeat::Combinations<NInputs, ArgTreeRefTupleT, TArgs...> mCombinations;
  mutable std::atomic_bool                                                mIsDirty = true;

  template<size_t N = 0>
  inline void initOutputs() const
  {
    if constexpr (NInputs + N < NArgs) {
      using TOut = std::tuple_element_t<NInputs + N, std::tuple<TArgs...>>;
      if constexpr (data::IsWriteView<TOut>::value) {
        std::get<N>(mOutputs).clear();
      }
      else {
        std::get<N>(mOutputs).resize(1);
      }
    }
    if constexpr (NInputs + N + 1 < NArgs) {
      initOutputs<N + 1>();
    }
  }

  // Runs the function.
  inline void run() const
  {
    mCombinations.init();
    initOutputs();
    if (!mCombinations.empty()) {
      do {
        std::apply(mFunc, mCombinations.template current<ArgTupleT>());
      } while (mCombinations.next());
    }
  }

private:
  // Calls update on all the upstream functions.
  inline void updateUpstream() const
  {
    std::apply([](const auto&... inputs) { (inputs.mOwner->update(), ...); }, mInputs);
  }

public:
  /**
   * @brief Creates a new instance of the function.
   *
   * @param fn The implementation of the function, either a function pointer or a lambda
   * expression. It should be a void function that accepts const references of all inputs
   * in order, followed by references of all outputs in order.
   * @param inputs The input registers.
   */
  TFunction(const ImplFuncType& fn, const InputRegTupleT& inputs)
      : mFunc(std::move(fn))
      , mOutputs()
      , mInputs(inputs)
      , mCombinations(makeArgTreeRefTuple<TArgList>(mInputs, mOutputs))
  {
    this->addSubscriber(mIsDirty);
  };

  virtual ~TFunction() {};

  void update() const override
  {
    if (mIsDirty) {
      if constexpr (HasInputs) {
        updateUpstream();
      }
      else {
        store::markDirty(this);
      }
      run();
      mIsDirty = false;
    }
  }

  void addSubscriber(std::atomic_bool& dirtyFlag) const override
  {
    if constexpr (HasInputs) {
      // Propagate the flag upstream.
      std::apply(
        [&dirtyFlag](const auto&... inputs) {
          (inputs.mOwner->addSubscriber(dirtyFlag), ...);
        },
        mInputs);
    }
    else {
      store::addSubscriber(this, dirtyFlag);
    }
  }

  PyOutputType pythonOutputRegs() const
  {
    if constexpr (NOutputs == 1) {
      return ArgRegisterT<typename TArgList::template Type<NInputs>>(
        dynamic_cast<const Function*>(this), &(std::get<0>(mOutputs)));
    }
    else {
      return pythonOutputTuple(this, mOutputs);
    }
  }

  template<size_t N>
  ArgRegisterT<typename TArgList::template Type<N + NInputs>> outputRegister()
  {
    return ArgRegisterT<typename TArgList::template Type<N + NInputs>>(
      this, &(std::get<N>(mOutputs)));
  }
};

/**
 * @brief Template for a variable functions.
 * @tparam TVal The type of the value stored in the variable.
 * @tparam TArgs... The type of arguments to be passed to the constructor of TVal to
 * initialize it.
 */
template<typename TVal, typename... TArgs>
struct TVariable : public TFunction<TVal>
{
  static constexpr bool sIsConstructible = std::is_constructible_v<TVal, TArgs...>;
  static constexpr bool IsSingleArgument = sizeof...(TArgs) == 1;

  static_assert(sIsConstructible || IsSingleArgument,
                "Cannot create variable with these arguments.");

  using TFirstArg    = typename std::tuple_element_t<0, std::tuple<TArgs...>>;
  using PyOutputType = ArgRegisterT<TVal>;

protected:
  inline uint64_t&         registerId() { return this->mRegIds[0]; }
  inline data::Tree<TVal>& tree() { return std::get<0>(this->mOutputs); };

  void setInternal(const TArgs&... args)
  {
    if constexpr (sIsConstructible) {
      tree().clear();
      tree().emplace_back(0, args...);
    }
    else if constexpr (IsSingleArgument) {
      tree().resize(1);
      Converter<TFirstArg, TVal>::assign(args..., tree().value(0));
    }
  }

  inline void run() const {/*Do Nothing.*/};

public:
  TVariable(const TArgs&... args)
      : TFunction<TVal>([](TVal&) -> void {}, {})
  {
    setInternal(args...);
  }

  void set(const TArgs&... args)
  {
    store::markDirty(this);
    setInternal(args...);
    this->mIsDirty = false;
  }
};

namespace store {

/**
 * @brief Creates a function instance that is tracked and managed.
 *
 * @tparam TFunc The function type.
 * @tparam TArgs The types of arguments to be passed to the constructor.
 * @param args The arguments to be passed to the constructor.
 */
template<typename TFunc, typename... TArgs>
std::shared_ptr<TFunc> makeFunction(const TArgs&... args)
{
  static_assert(std::is_base_of_v<Function, TFunc>, "Not a valid function type");

  auto fn = std::make_shared<TFunc>(args...);
  addFunction(std::dynamic_pointer_cast<Function>(fn));
  return fn;
};

}  // namespace store

template<typename TVal, typename... TArgs>
typename TVariable<TVal, TArgs...>::PyOutputType py_variable(const TArgs&... args)
{
  auto fn = std::make_shared<TVariable<TVal, TArgs...>>(args...);
  store::addFunction(std::dynamic_pointer_cast<Function>(fn));
  return fn->pythonOutputRegs();
};

template<typename T>
typename TVariable<std::vector<T>, boost::python::list>::PyOutputType py_list(
  const boost::python::list& lst)
{
  return py_variable<std::vector<T>, boost::python::list>(lst);
};

namespace python {

template<typename TFnPtr, typename... TFnPtrs>
void bindOverloads(const char* fnName, TFnPtr fn, TFnPtrs... rest)
{
  boost::python::def<TFnPtr>(fnName, fn);
  if constexpr (sizeof...(TFnPtrs) > 0) {
    bindOverloads<TFnPtrs...>(fnName, rest...);
  }
}

/**
 * @brief Gets the current python context in a path format. Must be used from inside a
 * function that has python binding. This can be used to find out in what context a
 * function was instantiated.
 *
 * @return fs::path This is not an actual file path. This is just the context (python
 * stack trace) represented as a path.
 */
fs::path getcontextpath();

}  // namespace python

}  // namespace func
}  // namespace gal

#define GAL_CONCAT(x, y) x##y
// Removes the braces from the type name at compile time.
#define GAL_UNBRACED_TYPE(type) typename gal::RemoveBraces<void(type)>::Type
// Get the type from an arg-tuple that has description.
#define _GAL_ARG_TYPE(type, name, desc) GAL_UNBRACED_TYPE(type)
#define GAL_ARG_TYPE(argTuple) _GAL_ARG_TYPE argTuple
// Get the const type from an arg-tuple with description.
#define _GAL_ARG_CONST_TYPE(type, name, desc) std::add_const_t<GAL_UNBRACED_TYPE(type)>
#define GAL_ARG_CONST_TYPE(argTuple) _GAL_ARG_CONST_TYPE argTuple
// Get the name from an arg-tuple with description.
#define _GAL_ARG_NAME(type, name, desc) name
#define GAL_ARG_NAME(argTuple) _GAL_ARG_NAME argTuple
// Expand types from a list of arg-tuples with description.
#define _GAL_EXPAND_TYPE_TUPLE(...) MAP_LIST(GAL_ARG_TYPE, __VA_ARGS__)
#define GAL_EXPAND_TYPE_TUPLE(types) _GAL_EXPAND_TYPE_TUPLE types
// Expand const types from a list of arg-tuples with description.
#define _GAL_EXPAND_CONST_TYPE_TUPLE(...) MAP_LIST(GAL_ARG_CONST_TYPE, __VA_ARGS__)
#define GAL_EXPAND_CONST_TYPE_TUPLE(types) _GAL_EXPAND_CONST_TYPE_TUPLE types
// Get reference type from an arg-tuple with description.
#define GAL_EXPAND_IMPL_ARG(argTuple) \
  typename gal::func::ImplFnArgType<GAL_ARG_TYPE(argTuple)>::Type GAL_ARG_NAME(argTuple)
// Get the const reference type from an arg-tuple with description.
#define GAL_EXPAND_IMPL_CONST_ARG(argTuple)                                           \
  typename gal::func::ImplFnArgType<GAL_ARG_CONST_TYPE(argTuple)>::Type GAL_ARG_NAME( \
    argTuple)
// Expand to a list of references from arg-tuples with description.
#define GAL_EXPAND_IMPL_ARGS(...) MAP_LIST(GAL_EXPAND_IMPL_ARG, __VA_ARGS__)
// Expand to a list of const references from arg-tuples with description.
#define GAL_EXPAND_IMPL_CONST_ARGS(...) MAP_LIST(GAL_EXPAND_IMPL_CONST_ARG, __VA_ARGS__)
// Get the register type for the python function argument.
#define GAL_PY_REGISTER_TYPE(typeTuple) \
  const gal::func::ArgRegisterT<GAL_ARG_TYPE(typeTuple)>&
// Get python register argument from an arg-tuple with description.
#define GAL_PY_REGISTER_ARG(typeTuple) \
  GAL_PY_REGISTER_TYPE(typeTuple) GAL_ARG_NAME(typeTuple)
// Get python argument register list from arg-tuples with descriptions.
#define GAL_EXPAND_PY_REGISTER_ARGS(...) MAP_LIST(GAL_PY_REGISTER_ARG, __VA_ARGS__)
// Get python argument type list.
#define GAL_EXPAND_PY_REGISTER_TYPES(...) MAP_LIST(GAL_PY_REGISTER_TYPE, __VA_ARGS__)
// Name of a function implementation.
#define GAL_FN_IMPL_NAME(fnName) GAL_CONCAT(fnName, _impl)
// Get register ids from arg-tuples without descriptions.
#define GAL_EXPAND_REG_NAMES(...) MAP_LIST(GAL_ARG_NAME, __VA_ARGS__)
// Actual declaration of a the static implementation of the function.
#define GAL_FN_IMPL(fnName, inputs, outputs)                              \
  static void GAL_FN_IMPL_NAME(fnName)(GAL_EXPAND_IMPL_CONST_ARGS inputs, \
                                       GAL_EXPAND_IMPL_ARGS       outputs)

// Declartion of a gal function.
#define GAL_FUNC(fnName, fnDesc, inputArgs, outputArgs)                               \
  GAL_FN_IMPL(fnName, inputArgs, outputArgs);                                         \
  static TFunction<GAL_EXPAND_CONST_TYPE_TUPLE(inputArgs),                            \
                   GAL_EXPAND_TYPE_TUPLE(outputArgs)>::PyOutputType                   \
    py_##fnName(GAL_EXPAND_PY_REGISTER_ARGS inputArgs)                                \
  {                                                                                   \
    using namespace gal::func;                                                        \
    using FType = TFunction<GAL_EXPAND_CONST_TYPE_TUPLE(inputArgs),                   \
                            GAL_EXPAND_TYPE_TUPLE(outputArgs)>;                       \
    auto fn     = store::makeFunction<FType>(                                         \
      GAL_FN_IMPL_NAME(fnName), std::make_tuple(GAL_EXPAND_REG_NAMES inputArgs)); \
    return fn->pythonOutputRegs();                                                    \
  };                                                                                  \
  static constexpr TFunction<GAL_EXPAND_CONST_TYPE_TUPLE(inputArgs),                  \
                             GAL_EXPAND_TYPE_TUPLE(outputArgs)>::                     \
    PyOutputType (*pyfnptr_##fnName)(GAL_EXPAND_PY_REGISTER_TYPES inputArgs) =        \
      &py_##fnName;                                                                   \
  GAL_FN_IMPL(fnName, inputArgs, outputArgs)

#define _GAL_TEMPL_PARAM_TYPE(argType, argSymbol) argType
#define GAL_TEMPL_PARAM_TYPE(templParam) _GAL_TEMPL_PARAM_TYPE templParam

#define _GAL_TEMPL_PARAM_SYMBOL(argType, argSymbol) argSymbol
#define GAL_TEMPL_PARAM_SYMBOL(templParam) _GAL_TEMPL_PARAM_SYMBOL templParam

#define GAL_TEMPL_PARAM(templateParam) \
  GAL_TEMPL_PARAM_TYPE(templateParam) GAL_TEMPL_PARAM_SYMBOL(templateParam)
#define GAL_EXPAND_TEMPL_PARAMS(...) MAP_LIST(GAL_TEMPL_PARAM, __VA_ARGS__)

#define GAL_TEMPL_ARG(templateParam) GAL_TEMPL_PARAM_SYMBOL(templateParam)
#define GAL_EXPAND_TEMPL_ARGS(...) MAP_LIST(GAL_TEMPL_ARG, __VA_ARGS__)

#define GAL_FUNC_TEMPLATE(templateParams, fnName, fnDesc, inputArgs, outputArgs) \
  template<GAL_EXPAND_TEMPL_PARAMS templateParams>                               \
  GAL_FN_IMPL(fnName, inputArgs, outputArgs);                                    \
  template<GAL_EXPAND_TEMPL_PARAMS templateParams>                               \
  static typename TFunction<GAL_EXPAND_CONST_TYPE_TUPLE(inputArgs),              \
                            GAL_EXPAND_TYPE_TUPLE(outputArgs)>::PyOutputType     \
    py_##fnName(GAL_EXPAND_PY_REGISTER_ARGS inputArgs)                           \
  {                                                                              \
    using namespace gal::func;                                                   \
    using FType = TFunction<GAL_EXPAND_CONST_TYPE_TUPLE(inputArgs),              \
                            GAL_EXPAND_TYPE_TUPLE(outputArgs)>;                  \
    auto fn     = store::makeFunction<FType>(                                    \
      GAL_FN_IMPL_NAME(fnName) < GAL_EXPAND_TEMPL_ARGS templateParams >,     \
      std::make_tuple(GAL_EXPAND_REG_NAMES inputArgs));                      \
    return fn->pythonOutputRegs();                                               \
  };                                                                             \
  template<GAL_EXPAND_TEMPL_PARAMS templateParams>                               \
  static constexpr typename TFunction<GAL_EXPAND_CONST_TYPE_TUPLE(inputArgs),    \
                                      GAL_EXPAND_TYPE_TUPLE(outputArgs)>::       \
    PyOutputType (*pyfnptr_##fnName)(GAL_EXPAND_PY_REGISTER_TYPES inputArgs) =   \
      &py_##fnName<GAL_EXPAND_TEMPL_ARGS templateParams>;                        \
  template<GAL_EXPAND_TEMPL_PARAMS templateParams>                               \
  GAL_FN_IMPL(fnName, inputArgs, outputArgs)

// Creates a python binding for the function.
#define _GAL_FN_BIND_ONE(fnName) boost::python::def(#fnName, pyfnptr_##fnName)
#define GAL_FN_BIND(...) MAP_LIST(_GAL_FN_BIND_ONE, __VA_ARGS__)

#define GAL_FN_PTR_VAR(fnName) pyfnptr_##fnName

#define GAL_FN_BIND_OVERLOADS(fnName, ...) \
  gal::func::python::bindOverloads(#fnName, MAP_LIST(GAL_FN_PTR_VAR, __VA_ARGS__))

#define GAL_FN_BIND_TEMPLATE(fnName, ...) \
  boost::python::def(#fnName, pyfnptr_##fnName<__VA_ARGS__>)

// Forward declaration of the module initializer, which will be defined by boost later.
// This should be called before running scripts from within C++.
extern "C" PyObject* PyInit_pygalfunc();
namespace gal {
namespace func {
namespace python {

/**
 * @brief Can be used from python to read the value inside a register. Only works if a
 * converter is available for the source datatype.
 *
 * @tparam T The source datatype (register).
 * @param reg The register.
 * @return boost::python::object Converted python object.
 */
template<typename T>
boost::python::object read(const Register<T>& reg)
{
  boost::python::object dst;
  Converter<data::Tree<T>, boost::python::object>::assign(reg.read(), dst);
  return dst;
}

/**
 * @brief Initializes the python bindings for register and other helper functions related
 * to the type T.
 *
 * @tparam T
 */
template<typename T>
struct defClass
{
  static boost::python::class_<Register<T>>& pythonType()
  {
    static const std::string name("r_" + TypeInfo<T>::name());
    static auto              sType = boost::python::class_<Register<T>>(name.c_str());
    return sType;
  }
  static void invoke()
  {
    // Defining the str function allows python to print the objects using the c++
    // implementation of the << operator.
    pythonType().def(boost::python::self_ns::str(boost::python::self_ns::self));
    // Read value into python if conversion is available.
    def("read", read<T>);
  }
};

}  // namespace python
}  // namespace func
}  // namespace gal

namespace std {

// Register printing to console
template<typename T>
std::ostream& operator<<(std::ostream& ostr, const gal::func::Register<T>& reg)
{
  ostr << "(" << gal::TypeInfo<T>::name() << " at " << reg.mData << ")";
  return ostr;
};

}  // namespace std
