#pragma once

#include <galfunc/Converter.h>
#include <galfunc/Functions.h>
#include <glm/glm.hpp>

namespace gal {
namespace func {

template<typename TVal, typename TArg = TVal>
struct TVariable : public Function
{
protected:
  uint64_t              mRegisterId;
  std::shared_ptr<TVal> mValuePtr;

public:
  TVariable(const TArg& value)
  {
    mValuePtr = Converter<TArg, TVal>::convert(value);
    store::useRegister(this, mRegisterId);
  };

  virtual ~TVariable() { store::free(mRegisterId); }

  void initOutputRegisters() override
  {
    static_assert(TypeInfo<TVal>::value, "Unknown type");
    mRegisterId = store::allocate(this, TypeInfo<TVal>::id, TypeInfo<TVal>::name());
    store::markDirty(this->mRegisterId);
  };

  size_t numOutputs() const override { return 1; };

  uint64_t outputRegister(size_t index) const override
  {
    if (index == 0) {
      return mRegisterId;
    }
    throw std::out_of_range("Index out of range");
  };

  void run() override { store::set<TVal>(mRegisterId, mValuePtr); };

  void set(const TArg& value)
  {
    Converter<TArg, TVal>::assign(value, *(this->mValuePtr));
    store::markDirty(this->mRegisterId);
  };
};

namespace store {

template<typename TVal, typename TArg = TVal>
std::shared_ptr<Function> makeVariable(const TArg& value)
{
  auto fn =
    std::dynamic_pointer_cast<Function>(std::make_shared<TVariable<TVal, TArg>>(value));
  return addFunction(fn);
};

}  // namespace store

template<typename TVal, typename TArg = TVal>
types::OutputTuple<1> variable(const TArg& value)
{
  auto fn = store::makeVariable<TVal, TArg>(value);
  return types::makeOutputTuple<1>(*fn);
};

template<typename TVal, typename TArg = TVal>
boost::python::tuple py_variable(const TArg& value)
{
  return pythonRegisterTuple(gal::func::variable<TVal, TArg>(value));
};

template<typename T>
boost::python::tuple py_list(const boost::python::list& lst)
{
  return py_variable<std::vector<T>, boost::python::list>(lst);
};

}  // namespace func
}  // namespace gal