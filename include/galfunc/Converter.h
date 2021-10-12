#pragma once

#include <galfunc/Data.h>
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
struct Converter<boost::python::list, glm::vec2>
{
  static std::shared_ptr<glm::vec2> convert(const boost::python::list& lst)
  {
    auto v = std::make_shared<glm::vec2>();
    assign(lst, *v);
    return v;
  }

  static void assign(const boost::python::list& src, glm::vec2& dst)
  {
    dst.x = boost::python::extract<float>(src[0]);
    dst.y = boost::python::extract<float>(src[1]);
  }
};

template<>
struct Converter<boost::python::list, glm::vec3>
{
  static std::shared_ptr<glm::vec3> convert(const boost::python::list& lst)
  {
    auto v = std::make_shared<glm::vec3>();
    assign(lst, *v);
    return v;
  }

  static void assign(const boost::python::list& src, glm::vec3& dst)
  {
    dst.x = boost::python::extract<float>(src[0]);
    dst.y = boost::python::extract<float>(src[1]);
    dst.z = boost::python::extract<float>(src[2]);
  }
};

template<>
struct Converter<boost::python::api::const_object_item, glm::vec3>
{
  static std::shared_ptr<glm::vec3> convert(
    const boost::python::api::const_object_item& obj)
  {
    auto v = std::make_shared<glm::vec3>();
    assign(obj, *v);
    return v;
  };

  static void assign(const boost::python::api::const_object_item& src, glm::vec3& dst)
  {
    boost::python::list lst = boost::python::extract<boost::python::list>(src);
    Converter<boost::python::list, glm::vec3>::assign(lst, dst);
  };
};

template<>
struct Converter<boost::python::api::const_object_item, glm::vec2>
{
  static std::shared_ptr<glm::vec2> convert(
    const boost::python::api::const_object_item& obj)
  {
    auto v = std::make_shared<glm::vec2>();
    assign(obj, *v);
    return v;
  };

  static void assign(const boost::python::api::const_object_item& src, glm::vec2& dst)
  {
    boost::python::list lst = boost::python::extract<boost::python::list>(src);
    Converter<boost::python::list, glm::vec2>::assign(lst, dst);
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
      boost::python::extract<boost::python::tuple>(tup));
  }

  static void assign(const boost::python::api::const_object_item& src,
                     std::pair<T1, T2>&                           dst)
  {
    Converter<boost::python::tuple, std::pair<T1, T2>>::assign(
      boost::python::extract<boost::python::tuple>(src), dst);
  }
};

template<typename T>
struct Converter<data::Tree<T>, boost::python::list>
{
private:
  using DepthT    = typename data::DepthT;
  using ValIter   = typename std::vector<T>::const_iterator;
  using DepthIter = typename std::vector<DepthT>::const_iterator;

  static void copyValues(ValIter&             vbegin,
                         const ValIter&       vend,
                         DepthIter&           dbegin,
                         boost::python::list& dst,
                         DepthT               cdepth = 0)
  {
    if (*dbegin == cdepth) {
      do {
        boost::python::object obj;
        Converter<T, boost::python::object>::assign(vbegin, obj);
        dst.append(obj);
        dbegin++;
        vbegin++;
      } while (*dbegin == 0 && vbegin != vend);
    }
    else if (*dbegin > cdepth) {
      DepthT ddiff = (*dbegin) - cdepth;
      do {
        boost::python::list lst;
        copyValues(vbegin, vend, dbegin, lst, cdepth + 1);
        dst.append(lst);
      } while ((*dbegin) - cdepth == ddiff);
    }
  }

public:
  static void assign(const data::Tree<T>& tree, boost::python::list& lst)
  {
    const auto& values = tree.values();
    const auto& depths = tree.depths();
    assert(values.size() == depths.size());
    auto vbegin = values.begin();
    auto vend   = values.end();
    auto dbegin = depths.begin();
    copyValues(vbegin, vend, dbegin, lst);
    assert(vbegin == vend);
  };
};

}  // namespace func
}  // namespace gal
