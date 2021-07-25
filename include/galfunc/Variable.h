#pragma once

#include <galfunc/Converter.h>
#include <galfunc/Functions.h>
#include <glm/glm.hpp>

namespace gal {
namespace func {

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

boost::python::object py_read(store::Register reg);

}  // namespace func
}  // namespace gal
