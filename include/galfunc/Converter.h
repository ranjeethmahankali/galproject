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
      dst = T2(src);
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

template<typename T>
struct Converter<boost::python::object, T>
{
  static std::shared_ptr<T> convert(const boost::python::object& obj)
  {
    return std::make_shared<T>(boost::python::extract<T>(obj));
  };

  static void assign(const boost::python::object& src, T& dst)
  {
    dst = boost::python::extract<T>(src);
  };
};

template<>
struct Converter<glm::vec3, boost::python::object>
{
  static void assign(const glm::vec3& src, boost::python::object& dst)
  {
    boost::python::list lst;
    lst.append(src[0]);
    lst.append(src[1]);
    lst.append(src[2]);
    dst = std::move(lst);
  }
};

template<>
struct Converter<glm::vec2, boost::python::object>
{
  static void assign(const glm::vec2& src, boost::python::object& dst)
  {
    boost::python::list lst;
    lst.append(src[0]);
    lst.append(src[1]);
    dst = std::move(lst);
  }
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

template<>
struct Converter<boost::python::api::const_object_item, glm::vec2>
{
  static std::shared_ptr<glm::vec2> convert(
    const boost::python::api::const_object_item& obj)
  {
    const boost::python::list& lst = (const boost::python::list&)obj;
    auto                       v   = std::make_shared<glm::vec2>();
    Converter<boost::python::api::const_object_item, float>::assign(lst[0], v->x);
    Converter<boost::python::api::const_object_item, float>::assign(lst[1], v->y);
    return v;
  };

  static void assign(const boost::python::api::const_object_item& src, glm::vec2& dst)
  {
    boost::python::list lst = boost::python::extract<boost::python::list>(src);
    dst.x                   = boost::python::extract<float>(lst[0]);
    dst.y                   = boost::python::extract<float>(lst[1]);
  };
};

template<>
struct Converter<boost::python::list, glm::vec2>
{
  static std::shared_ptr<glm::vec2> convert(const boost::python::list& lst)
  {
    return Converter<boost::python::api::const_object_item, glm::vec2>::convert(
      (const boost::python::api::const_object_item&)lst);
  }

  static void assign(const boost::python::list& src, glm::vec2& dst)
  {
    Converter<boost::python::api::const_object_item, glm::vec2>::assign(
      (const boost::python::api::const_object_item&)src, dst);
  }
};

template<>
struct Converter<boost::python::list, glm::vec3>
{
  static std::shared_ptr<glm::vec3> convert(const boost::python::list& lst)
  {
    return Converter<boost::python::api::const_object_item, glm::vec3>::convert(
      (const boost::python::api::const_object_item&)lst);
  }

  static void assign(const boost::python::list& src, glm::vec3& dst)
  {
    Converter<boost::python::api::const_object_item, glm::vec3>::assign(
      (const boost::python::api::const_object_item&)src, dst);
  }
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

template<typename T1, typename T2>
struct Converter<boost::python::tuple, std::pair<T1, T2>>
{
  static std::shared_ptr<std::pair<T1, T2>> convert(const boost::python::tuple& tup)
  {
    auto dst = std::make_shared<std::pair<T1, T2>>();
    assign(tup, *dst);
    return dst;
  }

  static void assign(const boost::python::tuple& src, std::pair<T1, T2>& dst)
  {
    Converter<boost::python::api::const_object_item, T1>::assign(src[0],
                                                                 std::get<0>(dst));
    Converter<boost::python::api::const_object_item, T2>::assign(src[1],
                                                                 std::get<1>(dst));
  }
};

template<typename T1, typename T2>
struct Converter<boost::python::api::const_object_item, std::pair<T1, T2>>
{
  static std::shared_ptr<std::pair<T1, T2>> convert(
    const boost::python::api::const_object_item& tup)
  {
    return Converter<boost::python::tuple, std::pair<T1, T2>>::convert(
      (const boost::python::tuple&)tup);
  }

  static void assign(const boost::python::api::const_object_item& src,
                     std::pair<T1, T2>&                           dst)
  {
    Converter<boost::python::tuple, std::pair<T1, T2>>::assign(
      (const boost::python::tuple&)src, dst);
  }
};

}  // namespace func
}  // namespace gal
