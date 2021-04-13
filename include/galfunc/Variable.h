#pragma once

#include <galfunc/Functions.h>

namespace gal {
namespace func {

template<typename T1, typename T2>
struct Converter
{
  static std::shared_ptr<T2> convert(const T1& val)
  {
    if constexpr (std::is_same_v<T1, T2>) {
      return std::make_shared<T1>(val);
    }
    else {
      T2 b = T2(val);
      return std::make_shared<T2>(std::move(b));
    }
  };

  static void assign(const T1& src, T2& dst)
  {
    if constexpr (std::is_same_v<T1, T2>) {
      dst = src;
    }
    else {
      dst = (T2)src;
    }
  };
};

template<typename T>
struct Converter<boost::python::list, std::vector<T>>
{
  static std::shared_ptr<std::vector<T>> convert(const boost::python::list& lst)
  {
    auto dst = std::make_shared<std::vector<T>>();
    assign(lst, *dst);
    return dst;
  };

  static void assign(const boost::python::list& src, std::vector<T>& dst)
  {
    size_t count = boost::python::len(src);
    dst.reserve(count);
    for (size_t i = 0; i < count; i++) {
      dst.push_back(boost::python::extract<T>(src[i]));
    }
  };
};

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
    static_assert(types::TypeInfo<TVal>::value, "Unknown type");
    mRegisterId =
      store::allocate(this, types::TypeInfo<TVal>::id, types::TypeInfo<TVal>::name());
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