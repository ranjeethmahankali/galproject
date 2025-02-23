#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

#include <spdlog/spdlog.h>

#include <Data.h>
#include <MapMacro.h>
#include <Property.h>
#include <TypeManager.h>
#include <Util.h>

namespace gal {
namespace func {

namespace python {

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

struct FuncInfo
{
  std::string_view        mName;
  std::string_view        mDesc;
  size_t                  mNumInputs;
  const std::string_view* mInputNames;
  const std::string_view* mInputDescriptions;

  size_t                  mNumOutputs;
  const std::string_view* mOutputNames;
  const std::string_view* mOutputDescriptions;
};

struct Function;

struct InputInfo
{
  const Function* mFunc;
  const int       mOutputIdx;

  InputInfo(const Function* func, const int outIdx)
      : mFunc(func)
      , mOutputIdx(outIdx)
  {}
};

spdlog::logger& logger();

/**
 * @brief Base class for all functions.
 */
struct Function
{
  // Virtual destructor because of polymorphism.
  virtual ~Function() = default;

  Function(Function const&)            = delete;
  Function(Function&&)                 = delete;
  Function& operator=(Function const&) = delete;
  Function& operator=(Function&&)      = delete;

  /**
   * @brief Ensures the output data is up to date.
   */
  virtual void update() const = 0;

  virtual void addSubscriber(bool& dirtyFlag) const = 0;

  const fs::path& contextpath() const;

  const FuncInfo& info() const;
  FuncInfo&       info();

  size_t numInputs() const;

  size_t numOutputs() const;

  virtual void getInputs(std::vector<InputInfo>& dst) const = 0;

  int  index() const;
  int& index();

protected:
  Function();

private:
  fs::path mContextPath;
  FuncInfo mInfo;
  int      mIndex = -1;
};

namespace store {

/**
 * @brief Adds the function instance to the list of managed functions. This is necessary
 * because all function instances must be tracked and managed.
 *
 * @param fn
 */
Function* addFunction(const FuncInfo& fnInfo, std::unique_ptr<Function> fn);

/**
 * @brief Gets the number of functions in the current session.
 *
 * @return size_t
 */
size_t numFunctions();

/**
 * @brief Get the function with the given index.
 *
 * @param i Index.
 */
const Function& function(size_t i);

/**
 * @brief Properties container for the functions in the current session.
 */
Properties& properties();

/**
 * @brief Create a new property for the functions in the current session.
 *
 * @tparam T
 */
template<typename T>
Property<T> addProperty()
{
  Property<T> p(properties());
  properties().resize(numFunctions());
  return p;
}

/**
 * @brief Unloads all loaded function instances.
 */
void unloadAllFunctions();

};  // namespace store

/**
 * @brief Wrapper that points to readonly data owned by a
 * function. This is an output of that function. These are passed down
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
  size_t               mIndex = 0;  // Index of the output.

  Register() = default;

  Register(const Function* fn, const data::Tree<T>* dref, size_t index)
      : mOwner(fn)
      , mData(dref)
      , mIndex(index)
  {}

  const data::Tree<T>& read() const
  {
    mOwner->update();
    return *mData;
  }

  const Function* owner() const { return mOwner; }
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
  using FnPtrType = void (*)(typename ImplFnArgType<TArgs>::Type...);

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
py::tuple pythonOutputTupleInternal(const Function*     fn,
                                    const OutputTupleT& src,
                                    std::index_sequence<Is...>)
{
  return py::make_tuple(
    ArgRegisterT<typename std::tuple_element_t<Is, OutputTupleT>::Type>(
      fn, &(std::get<Is>(src)), Is)...);
}

/**
 * @brief Wraps the output tuple of the function in registers and returns a python tuple
 * of registers.
 *
 * @tparam OutputTupleT The output tuple type.
 * @param fn The function that owns the outputs.
 * @param src The output tuple. It is the source of the wrapped register tuple.
 * @return py::tuple
 */
template<typename OutputTupleT>
py::tuple pythonOutputTuple(const Function* fn, const OutputTupleT& src)
{
  return pythonOutputTupleInternal(
    fn, src, std::make_index_sequence<std::tuple_size_v<OutputTupleT>> {});
}

template<typename... Ts>
struct EmptyCallable
{
  void operator()(typename ImplFnArgType<Ts>::Type...) const {};
};

/**
 * @brief Template for all non-variable functions.
 * @tparam NInputs Number of inputs.
 * @tparam TArgs All arguments. Inputs followed by outputs.
 */
template<typename TCallable, typename... TArgs>
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
  static constexpr bool   HasInputs   = NInputs > 0;
  static constexpr size_t NOutputs    = NArgs - NInputs;
  static constexpr bool   NOutputsGT1 = NOutputs > 1;
  static constexpr bool   NOutputsGT0 = NOutputs > 0;
  static constexpr bool   NOutputsEQ0 = NOutputs == 0;

  using PyOutputType = std::conditional_t<
    NOutputsEQ0,
    void,
    std::conditional_t<
      NOutputsGT1,
      py::tuple,
      /* The ternary expression is just in case NOutputs is zero, to make
         sure the code will still compile. It should never lead to a wrong type index
         being used because NOutputs == 0 case is already handled above. */
      ArgRegisterT<typename TArgList::template Type<(NOutputsGT0 ? NInputs : 0)>>>>;

  using ExpirationT =
    std::conditional_t<HasInputs, bool, std::vector<std::reference_wrapper<bool>>>;

private:
  /* Some fields are marked mutable because the function is considered changed only if the
   * inputs are changed. Running the function, which changes the output, or the status of
   * the dirty-flag, is not considered changing.
   */
  TCallable                                                               mFunc;
  mutable OutputTupleT                                                    mOutputs;
  InputRegTupleT                                                          mInputs;
  mutable data::repeat::Combinations<NInputs, ArgTreeRefTupleT, TArgs...> mCombinations;
  mutable ExpirationT                                                     mExpiration;

  template<size_t N = 0>
  inline void clearOutputs() const
  {
    if constexpr (NInputs + N < NArgs) {
      std::get<N>(mOutputs).clear();
    }
    if constexpr (NInputs + N + 1 < NArgs) {
      clearOutputs<N + 1>();
    }
  }

  // Runs the function.
  inline void run() const
  {
    if constexpr (!IsInstance<EmptyCallable, TCallable>::value) {
      clearOutputs();
      mCombinations.init();
      if (!mCombinations.empty()) {
        do {
          std::apply(mFunc, mCombinations.template current<ArgTupleT>());
        } while (mCombinations.next());
      }
    }
  }

  // Calls update on all the upstream functions.
  inline void updateUpstream() const
  {
    std::apply([](const auto&... inputs) { (inputs.mOwner->update(), ...); }, mInputs);
  }

protected:
  OutputTupleT& outputs() { return mOutputs; }

public:
  /**
   * @brief Creates a new instance of the function.
   *
   * @param fn The implementation of the function, either a function pointer or a lambda
   * expression. It should be a void function that accepts const references of all inputs
   * in order, followed by references of all outputs in order.
   * @param inputs The input registers.
   */
  TFunction(const TCallable& fn, const InputRegTupleT& inputs)
      : mFunc(std::move(fn))
      , mOutputs()
      , mInputs(inputs)
      , mCombinations(makeArgTreeRefTuple<TArgList>(mInputs, mOutputs))
  {
    if constexpr (HasInputs) {
      mExpiration = true;
      this->addSubscriber(mExpiration);
    }
  };

  virtual ~TFunction() = default;

  TFunction(TFunction const&)            = delete;
  TFunction(TFunction&&)                 = delete;
  TFunction& operator=(TFunction const&) = delete;
  TFunction& operator=(TFunction&&)      = delete;

  void expire() const
  {
    if constexpr (!HasInputs) {
      // Expire all downstream functions.
      for (bool& flag : mExpiration) {
        // cppcheck-suppress useStlAlgorithm
        flag = true;
      }
    }
  }

  void unexpire() const
  {
    if constexpr (HasInputs) {
      mExpiration = false;
    }
  }

  bool isExpired() const
  {
    if constexpr (HasInputs) {
      return mExpiration;
    }
    else {
      return false;
    }
  }

  void update() const override
  {
    if (isExpired()) {
      updateUpstream();
      run();
      unexpire();
    }
  }

  void addSubscriber(bool& flag) const override
  {
    if constexpr (HasInputs) {
      // Propagate the flag upstream.
      std::apply(
        [&flag](const auto&... inputs) { (inputs.mOwner->addSubscriber(flag), ...); },
        mInputs);
    }
    else {
      mExpiration.push_back(flag);
    }
  }

  void getInputs(std::vector<InputInfo>& dst) const override
  {
    dst.clear();
    if constexpr (HasInputs) {
      std::apply(
        [&dst](const auto&... inputs) {
          (dst.emplace_back(inputs.mOwner, inputs.mIndex), ...);
        },
        mInputs);
    }
  }

  PyOutputType pythonOutputRegs() const
  {
    if constexpr (NOutputs == 1) {
      return ArgRegisterT<typename TArgList::template Type<NInputs>>(
        dynamic_cast<const Function*>(this), &(std::get<0>(mOutputs)), 0);
    }
    else if constexpr (NOutputs == 0) {
      return;
    }
    else {
      return pythonOutputTuple(this, mOutputs);
    }
  }

  template<size_t N>
  ArgRegisterT<typename TArgList::template Type<N + NInputs>> outputRegister()
  {
    return ArgRegisterT<typename TArgList::template Type<N + NInputs>>(
      this, &(std::get<N>(mOutputs)), N);
  }
};

template<typename... TArgs>
using TFunctionWithFnPtr = TFunction<typename TypeList<TArgs...>::FnPtrType, TArgs...>;

/**
 * @brief Template for a variable functions.
 * @tparam TVal The type of the value stored in the variable.
 * @tparam TArgs... The type of arguments to be passed to the constructor of TVal to
 * initialize it.
 */
template<typename TVal>
struct TVariable : public TFunction<EmptyCallable<TVal>, TVal>
{
  using PyOutputType = ArgRegisterT<TVal>;
  using BaseT        = TFunction<EmptyCallable<TVal>, TVal>;

protected:
  inline uint64_t&         registerId() { return this->mRegIds[0]; }
  inline data::Tree<TVal>& tree() { return std::get<0>(this->outputs()); };

  void setInternal(const py::object& obj)
  {
    Converter<py::object, data::Tree<TVal>>::assign(obj, tree());
  }

  void setInternal(const TVal& val)
  {
    tree().resize(1);
    tree().value(0) = val;
  }

public:
  explicit TVariable(const py::object& obj)
      : BaseT({}, {})
  {
    setInternal(obj);
  }

  explicit TVariable(const TVal& val)
      : BaseT({}, {})
  {
    setInternal(val);
  }

  TVariable()
      : BaseT({}, {})
  {}

  virtual ~TVariable() = default;

  TVariable(TVariable const&)            = delete;
  TVariable(TVariable&&)                 = delete;
  TVariable& operator=(TVariable const&) = delete;
  TVariable& operator=(TVariable&&)      = delete;

  void set(const py::object& obj)
  {
    this->expire();
    setInternal(obj);
    this->unexpire();
  }

  void set(const TVal& val)
  {
    this->expire();
    setInternal(val);
    this->unexpire();
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
TFunc* makeFunction(const FuncInfo& fnInfo, const TArgs&... args)
{
  static_assert(std::is_base_of_v<Function, TFunc>, "Not a valid function type");
  return dynamic_cast<TFunc*>(
    store::addFunction(fnInfo, std::make_unique<TFunc>(args...)));
};

}  // namespace store

template<typename T>
FuncInfo varfnInfo()
{
  static const std::string sName = "var_" + TypeInfo<T>::name();
  static const std::string sDesc = "Variable of type " + TypeInfo<T>::name() + ".";
  static const auto sOutputNames = gal::utils::makeArray<std::string_view>("value");
  static const auto sOutputDesc =
    gal::utils::makeArray<std::string_view>("The value output of the variable");

  return {{sName.data(), sName.size()},
          {sDesc.data(), sDesc.size()},
          0,
          nullptr,
          nullptr,
          1,
          sOutputNames.data(),
          sOutputDesc.data()};
}

template<typename TVal>
typename TVariable<TVal>::PyOutputType py_varWithValue(const py::object& obj)
{
  static const auto sInfo = varfnInfo<TVal>();
  auto              fn    = dynamic_cast<const TVariable<TVal>*>(
    store::addFunction(sInfo, std::make_unique<TVariable<TVal>>(obj)));
  return fn->pythonOutputRegs();
};

template<typename TVal>
typename TVariable<TVal>::PyOutputType py_varEmpty()
{
  static const auto sInfo = varfnInfo<TVal>();
  auto              fn    = dynamic_cast<const TVariable<TVal>*>(
    store::addFunction(sInfo, std::make_unique<TVariable<TVal>>()));
  return fn->pythonOutputRegs();
}

namespace python {

/**
 * @brief Python doc-string of a function.
 *
 */
struct FuncDocString
{
  /**
   * @brief Create a new python doc-string.
   *
   * @param desc Description of the function.
   * @param inputs Names and descriptions of inputs.
   * @param outputs Names and descriptions of outputs.
   */
  explicit FuncDocString(const FuncInfo& finfo);

  /**
   * @brief Gets the c-string representation of the python doc-string.
   *
   * @return char* c-string.
   */
  const char* c_str() const;

private:
  std::string mDocString;
};

template<typename TFnPtr, typename... TFnPtrs>
void bindOverloads(py::module&                             m,
                   const char*                             fnName,
                   std::pair<TFnPtr, const FuncDocString*> fn,
                   std::pair<TFnPtrs, const FuncDocString*>... rest)
{
  m.def(fnName, std::get<0>(fn), std::get<1>(fn)->c_str());
  if constexpr (sizeof...(TFnPtrs) > 0) {
    bindOverloads<TFnPtrs...>(m, fnName, rest...);
  }
}

}  // namespace python

}  // namespace func
}  // namespace gal

#define GAL_CONCAT(x, y) x##y  // NOLINT
// Removes the braces from the type name at compile time.

#define GAL_UNBRACED_TYPE(type) DEPAREN(type)  // NOLINT
// Get the type from an arg-tuple that has description.
#define _GAL_ARG_TYPE(type, name, desc) GAL_UNBRACED_TYPE(type)  // NOLINT
#define GAL_ARG_TYPE(argTuple) _GAL_ARG_TYPE argTuple            // NOLINT
// Get the const type from an arg-tuple.
// NOLINTNEXTLINE
#define _GAL_ARG_CONST_TYPE(type, name, desc) std::add_const_t<GAL_UNBRACED_TYPE(type)>
#define GAL_ARG_CONST_TYPE(argTuple) _GAL_ARG_CONST_TYPE argTuple  // NOLINT
// Get the name from an arg-tuple.
#define _GAL_ARG_NAME(type, name, desc) name           // NOLINT
#define GAL_ARG_NAME(argTuple) _GAL_ARG_NAME argTuple  // NOLINT
// Get the description from an arg-tuple.
#define _GAL_ARG_DESC(type, name, desc) desc  // NOLINT
#define GAL_ARG_DESC(tup) _GAL_ARG_DESC tup   // NOLINT
// Expand types from a list of arg-tuples.
#define _GAL_EXPAND_TYPE_TUPLE(...) MAP_LIST(GAL_ARG_TYPE, __VA_ARGS__)  // NOLINT
#define GAL_EXPAND_TYPE_TUPLE(types) _GAL_EXPAND_TYPE_TUPLE types        // NOLINT
// Expand const types from a list of arg-tuples.
// NOLINTNEXTLINE
#define _GAL_EXPAND_CONST_TYPE_TUPLE(...) MAP_LIST(GAL_ARG_CONST_TYPE, __VA_ARGS__)
#define GAL_EXPAND_CONST_TYPE_TUPLE(types) _GAL_EXPAND_CONST_TYPE_TUPLE types  // NOLINT
// Get reference type from an arg-tuple.
// NOLINTNEXTLINE
#define GAL_EXPAND_IMPL_ARG(argTuple) \
  typename gal::func::ImplFnArgType<GAL_ARG_TYPE(argTuple)>::Type GAL_ARG_NAME(argTuple)
// Get the const reference type from an arg-tuple.
// NOLINTNEXTLINE
#define GAL_EXPAND_IMPL_CONST_ARG(argTuple)                                           \
  typename gal::func::ImplFnArgType<GAL_ARG_CONST_TYPE(argTuple)>::Type GAL_ARG_NAME( \
    argTuple)
// Expand to a list of references from arg-tuples.
#define GAL_EXPAND_IMPL_ARGS(...) MAP_LIST(GAL_EXPAND_IMPL_ARG, __VA_ARGS__)  // NOLINT
// Expand to a list of const references from arg-tuples.
// NOLINTNEXTLINE
#define GAL_EXPAND_IMPL_CONST_ARGS(...) MAP_LIST(GAL_EXPAND_IMPL_CONST_ARG, __VA_ARGS__)
// Get the register type for the python function argument.
// NOLINTNEXTLINE
#define GAL_PY_REGISTER_TYPE(typeTuple) \
  const gal::func::ArgRegisterT<GAL_ARG_TYPE(typeTuple)>&
// Get python register argument from an arg-tuple.
// NOLINTNEXTLINE
#define GAL_PY_REGISTER_ARG(typeTuple) \
  GAL_PY_REGISTER_TYPE(typeTuple) GAL_ARG_NAME(typeTuple)
// Get python argument register list from arg-tuples.
// NOLINTNEXTLINE
#define GAL_EXPAND_PY_REGISTER_ARGS(...) MAP_LIST(GAL_PY_REGISTER_ARG, __VA_ARGS__)
// Get python argument type list.
// NOLINTNEXTLINE
#define GAL_EXPAND_PY_REGISTER_TYPES(...) MAP_LIST(GAL_PY_REGISTER_TYPE, __VA_ARGS__)
// Name of a function implementation.
#define GAL_FN_IMPL_NAME(fnName) GAL_CONCAT(fnName, _impl)  // NOLINT
// Get register ids from arg-tuples.
#define GAL_EXPAND_REG_NAMES(...) MAP_LIST(GAL_ARG_NAME, __VA_ARGS__)  // NOLINT
// Actual declaration of a the static implementation of the function.
// NOLINTNEXTLINE
#define GAL_FN_IMPL(fnName, inputs, outputs)                              \
  static void GAL_FN_IMPL_NAME(fnName)(GAL_EXPAND_IMPL_CONST_ARGS inputs, \
                                       GAL_EXPAND_IMPL_ARGS       outputs)

// Gets the documentation info of an argument.
// NOLINTNEXTLINE
#define _GAL_ARG_INFO(type, argname, desc) \
  {                                        \
#argname, desc                         \
  }
#define GAL_ARG_INFO(arg) _GAL_ARG_INFO arg  // NOLINT
// Expand the name and description of arguments
#define _GAL_EXPAND_ARG_INFOS(...) MAP_LIST(GAL_ARG_INFO, __VA_ARGS__)  // NOLINT
#define GAL_EXPAND_ARG_INFOS(args) _GAL_EXPAND_ARG_INFOS args           // NOLINT

// Expands the names of arguments as c-strings.
#define _GAL_ARG_NAME_STR(type, argname, desc) #argname  // NOLINT
#define GAL_ARG_NAME_STR(arg) _GAL_ARG_NAME_STR arg      // NOLINT
// Expand the names of arguments as c-strings.
#define _GAL_EXPAND_ARG_NAMES(...) MAP_LIST(GAL_ARG_NAME_STR, __VA_ARGS__)  // NOLINT
#define GAL_EXPAND_ARG_NAMES(args) _GAL_EXPAND_ARG_NAMES args               // NOLINT
// Expand the descriptions of the arguments.
#define _GAL_EXPAND_ARG_DESCS(...) MAP_LIST(GAL_ARG_DESC, __VA_ARGS__)  // NOLINT
#define GAL_EXPAND_ARG_DESCS(args) _GAL_EXPAND_ARG_DESCS args           // NOLINT

// Documentation info of the function
// NOLINTNEXTLINE
#define GAL_PY_FN_DOC_STR(fnName)       \
  static const auto pyfnInfo_##fnName = \
    gal::func::python::FuncDocString(sFnInfo_##fnName);

// NOLINTNEXTLINE
#define GAL_FN_INFO_DECL(fnName, fnDesc, inputArgs, outputArgs)                \
  static auto sInputNames_##fnName =                                           \
    gal::utils::makeArray<std::string_view>(GAL_EXPAND_ARG_NAMES(inputArgs));  \
  static auto sInputDescriptions_##fnName =                                    \
    gal::utils::makeArray<std::string_view>(GAL_EXPAND_ARG_DESCS(inputArgs));  \
  static auto sOutputNames_##fnName =                                          \
    gal::utils::makeArray<std::string_view>(GAL_EXPAND_ARG_NAMES(outputArgs)); \
  static auto sOutputDescriptions_##fnName =                                   \
    gal::utils::makeArray<std::string_view>(GAL_EXPAND_ARG_DESCS(outputArgs)); \
  static const gal::func::FuncInfo sFnInfo_##fnName = {                        \
    #fnName,                                                                   \
    fnDesc,                                                                    \
    sInputNames_##fnName.size(),                                               \
    sInputNames_##fnName.data(),                                               \
    sInputDescriptions_##fnName.data(),                                        \
    sOutputNames_##fnName.size(),                                              \
    sOutputNames_##fnName.data(),                                              \
    sOutputDescriptions_##fnName.data()};

// Declartion of a gal function.
// NOLINTNEXTLINE
#define GAL_FUNC(fnName, fnDesc, inputArgs, outputArgs)                                  \
  GAL_FN_INFO_DECL(fnName, fnDesc, inputArgs, outputArgs);                               \
  GAL_PY_FN_DOC_STR(fnName)                                                              \
  GAL_FN_IMPL(fnName, inputArgs, outputArgs);                                            \
  static gal::func::TFunctionWithFnPtr<GAL_EXPAND_CONST_TYPE_TUPLE(inputArgs),           \
                                       GAL_EXPAND_TYPE_TUPLE(outputArgs)>::PyOutputType  \
    py_##fnName(GAL_EXPAND_PY_REGISTER_ARGS inputArgs)                                   \
  {                                                                                      \
    using namespace gal::func;                                                           \
    using FType = TFunctionWithFnPtr<GAL_EXPAND_CONST_TYPE_TUPLE(inputArgs),             \
                                     GAL_EXPAND_TYPE_TUPLE(outputArgs)>;                 \
    auto fn =                                                                            \
      store::makeFunction<FType>(sFnInfo_##fnName,                                       \
                                 GAL_FN_IMPL_NAME(fnName),                               \
                                 std::make_tuple(GAL_EXPAND_REG_NAMES inputArgs));       \
    return fn->pythonOutputRegs();                                                       \
  };                                                                                     \
  static constexpr gal::func::TFunctionWithFnPtr<GAL_EXPAND_CONST_TYPE_TUPLE(inputArgs), \
                                                 GAL_EXPAND_TYPE_TUPLE(outputArgs)>::    \
    PyOutputType (*pyfnptr_##fnName)(GAL_EXPAND_PY_REGISTER_TYPES inputArgs) =           \
      &py_##fnName;                                                                      \
  GAL_FN_IMPL(fnName, inputArgs, outputArgs)

#define _GAL_TEMPL_PARAM_TYPE(argType, argSymbol) argType                  // NOLINT
#define GAL_TEMPL_PARAM_TYPE(templParam) _GAL_TEMPL_PARAM_TYPE templParam  // NOLINT

#define _GAL_TEMPL_PARAM_SYMBOL(argType, argSymbol) argSymbol                  // NOLINT
#define GAL_TEMPL_PARAM_SYMBOL(templParam) _GAL_TEMPL_PARAM_SYMBOL templParam  // NOLINT

// NOLINTNEXTLINE
#define GAL_TEMPL_PARAM(templateParam) \
  GAL_TEMPL_PARAM_TYPE(templateParam) GAL_TEMPL_PARAM_SYMBOL(templateParam)
#define GAL_EXPAND_TEMPL_PARAMS(...) MAP_LIST(GAL_TEMPL_PARAM, __VA_ARGS__)  // NOLINT

#define GAL_TEMPL_ARG(templateParam) GAL_TEMPL_PARAM_SYMBOL(templateParam)  // NOLINT
#define GAL_EXPAND_TEMPL_ARGS(...) MAP_LIST(GAL_TEMPL_ARG, __VA_ARGS__)     // NOLINT

// NOLINTNEXTLINE
#define GAL_FUNC_TEMPLATE(tparams, fnName, fnDesc, inputArgs, outputArgs)               \
  GAL_FN_INFO_DECL(fnName, fnDesc, inputArgs, outputArgs);                              \
  GAL_PY_FN_DOC_STR(fnName)                                                             \
  template<GAL_EXPAND_TEMPL_PARAMS tparams>                                             \
  GAL_FN_IMPL(fnName, inputArgs, outputArgs);                                           \
  template<GAL_EXPAND_TEMPL_PARAMS tparams>                                             \
  static typename gal::func::TFunctionWithFnPtr<GAL_EXPAND_CONST_TYPE_TUPLE(inputArgs), \
                                                GAL_EXPAND_TYPE_TUPLE(outputArgs)>::    \
    PyOutputType py_##fnName(GAL_EXPAND_PY_REGISTER_ARGS inputArgs)                     \
  {                                                                                     \
    using namespace gal::func;                                                          \
    using FType = TFunctionWithFnPtr<GAL_EXPAND_CONST_TYPE_TUPLE(inputArgs),            \
                                     GAL_EXPAND_TYPE_TUPLE(outputArgs)>;                \
    auto fn     = store::makeFunction<FType>(                                           \
      sFnInfo_##fnName,                                                             \
      GAL_FN_IMPL_NAME(fnName) < GAL_EXPAND_TEMPL_ARGS tparams >,                   \
      std::make_tuple(GAL_EXPAND_REG_NAMES inputArgs));                             \
    return fn->pythonOutputRegs();                                                      \
  };                                                                                    \
  template<GAL_EXPAND_TEMPL_PARAMS tparams>                                             \
  static constexpr                                                                      \
    typename gal::func::TFunctionWithFnPtr<GAL_EXPAND_CONST_TYPE_TUPLE(inputArgs),      \
                                           GAL_EXPAND_TYPE_TUPLE(outputArgs)>::         \
      PyOutputType (*pyfnptr_##fnName)(GAL_EXPAND_PY_REGISTER_TYPES inputArgs) =        \
        &py_##fnName<GAL_EXPAND_TEMPL_ARGS tparams>;                                    \
  template<GAL_EXPAND_TEMPL_PARAMS tparams>                                             \
  GAL_FN_IMPL(fnName, inputArgs, outputArgs)

// Creates a python binding for the function.
// NOLINTNEXTLINE
#define GAL_FN_BIND(fnName, module) \
  module.def(#fnName, pyfnptr_##fnName, pyfnInfo_##fnName.c_str())

// NOLINTNEXTLINE
#define _GAL_FN_OVERLOAD_DATA(fnName) std::make_pair(pyfnptr_##fnName, &pyfnInfo_##fnName)
// NOLINTNEXTLINE
#define GAL_FN_BIND_OVERLOADS(module, fnName, ...) \
  gal::func::python::bindOverloads(                \
    module, #fnName, MAP_LIST(_GAL_FN_OVERLOAD_DATA, __VA_ARGS__))
// NOLINTNEXTLINE
#define GAL_FN_BIND_TEMPLATE(fnName, module, ...) \
  module.def(#fnName, pyfnptr_##fnName<__VA_ARGS__>, pyfnInfo_##fnName.c_str())

// Forward declaration of the module initializer, which will be defined by boost later.
// This should be called before running scripts from within C++.
#ifdef _MSC_VER
extern "C" __declspec(dllexport) PyObject* PyInit_pygalfunc();
#else
extern "C" PyObject* PyInit_pygalfunc();
#endif
namespace gal {
namespace func {
namespace python {

/**
 * @brief Can be used from python to read the value inside a register. Only works if a
 * converter is available for the source datatype.
 *
 * @tparam T The source datatype (register).
 * @param reg The register.
 * @return py::object Converted python object.
 */
template<typename T>
py::object read(const Register<T>& reg)
{
  py::object dst;
  Converter<data::Tree<T>, py::object>::assign(reg.read(), dst);
  return dst;
}

template<typename T>
void assign(const Register<T>& reg, const py::object& src)
{
  const TVariable<T>* mOwner = dynamic_cast<const TVariable<T>*>(reg.mOwner);
  if (mOwner) {
    const_cast<TVariable<T>*>(mOwner)->set(src);  // NOLINT
  }
  else {
    throw std::runtime_error(
      "Cannot assign to this register because it's not owned by a variable");
  }
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
  static py::class_<Register<T>>& pythonType(py::module& mod)
  {
    static const std::string name("r_" + TypeInfo<T>::name());
    static auto              sType = py::class_<Register<T>>(mod, name.c_str());
    return sType;
  }

  static void invoke(py::module& mod)
  {
    // Defining the str function allows python to print the objects using the c++
    // implementation of the << operator.
    pythonType(mod).def("__str__", [](const Register<T>& reg) {
      std::stringstream ss;
      ss << reg;
      return ss.str();
    });
    // Functions to create a varable of the type.
    static const FuncInfo    varInfo = varfnInfo<T>();
    static const std::string sVarDesc =
      "Create a variable of type " + TypeInfo<T>::name() + " with the given value.";
    static const std::string sVarEmptyDesc =
      "Create an empty variable of the type " + TypeInfo<T>::name() + ".";

    mod.def(varInfo.mName.data(), py_varWithValue<T>, sVarDesc.c_str());
    mod.def(varInfo.mName.data(), py_varEmpty<T>, sVarEmptyDesc.c_str());
    // Read value into python if conversion is available.
    mod.def(
      "read",
      read<T>,
      "Read the data from the variable. This might cause all or some of the upstream "
      "functions to be evaluated.");
    // Assign values to registers.
    mod.def("assign", assign<T>, "Assign the given value to the variable");
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
