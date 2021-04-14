#pragma once

#include <galfunc/Functions.h>
#include <glm/glm.hpp>

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
struct Converter<boost::python::api::const_object_item, T>
{
  static std::shared_ptr<T> convert(const boost::python::api::const_object_item& obj)
  {
    return std::make_shared<T>(boost::python::extract<T>(obj));
  };

  static void assign(const boost::python::api::const_object_item& src, T& dst)
  {
    dst = boost::python::extract<T>(src);
  };
};

template<>
struct Converter<boost::python::api::const_object_item, glm::vec3>
{
  static std::shared_ptr<glm::vec3> convert(
    const boost::python::api::const_object_item& obj)
  {
    const boost::python::list& lst = (const boost::python::list&)obj;
    auto                       v   = std::make_shared<glm::vec3>();
    Converter<boost::python::api::const_object_item, float>::assign(lst[0], v->x);
    Converter<boost::python::api::const_object_item, float>::assign(lst[1], v->y);
    Converter<boost::python::api::const_object_item, float>::assign(lst[2], v->z);
    return v;
  };

  static void assign(const boost::python::api::const_object_item& src, glm::vec3& dst)
  {
    boost::python::list lst = boost::python::extract<boost::python::list>(src);
    dst.x                   = boost::python::extract<float>(lst[0]);
    dst.y                   = boost::python::extract<float>(lst[1]);
    dst.z                   = boost::python::extract<float>(lst[2]);
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
    dst.resize(count);
    for (size_t i = 0; i < count; i++) {
      Converter<boost::python::api::const_object_item, T>::assign(src[i], dst.at(i));
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