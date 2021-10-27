#pragma once

#include <galfunc/Data.h>
#include <galfunc/Functions.h>
#include <glm/glm.hpp>

namespace gal {
namespace func {

template<typename T1, typename T2>
struct Converter
{
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
  static void assign(const boost::python::api::const_object_item& src, T& dst)
  {
    dst = boost::python::extract<T>(src);
  };
};

template<typename T>
struct Converter<boost::python::object, T>
{
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
    dst = boost::python::make_tuple(src[0], src[1], src[2]);
  }
};

template<>
struct Converter<glm::vec2, boost::python::object>
{
  static void assign(const glm::vec2& src, boost::python::object& dst)
  {
    dst = boost::python::make_tuple(src[0], src[1]);
  }
};

template<>
struct Converter<boost::python::tuple, glm::vec2>
{
  static void assign(const boost::python::tuple& src, glm::vec2& dst)
  {
    dst.x = boost::python::extract<float>(src[0]);
    dst.y = boost::python::extract<float>(src[1]);
  }
};

template<>
struct Converter<boost::python::tuple, glm::vec3>
{
  static void assign(const boost::python::tuple& src, glm::vec3& dst)
  {
    dst.x = boost::python::extract<float>(src[0]);
    dst.y = boost::python::extract<float>(src[1]);
    dst.z = boost::python::extract<float>(src[2]);
  }
};

template<>
struct Converter<boost::python::object, glm::vec3>
{
  static void assign(const boost::python::object& src, glm::vec3& dst)
  {
    boost::python::tuple tup = boost::python::extract<boost::python::tuple>(src);
    Converter<boost::python::tuple, glm::vec3>::assign(tup, dst);
  };
};

template<>
struct Converter<boost::python::api::object, glm::vec2>
{
  static void assign(const boost::python::object& src, glm::vec2& dst)
  {
    boost::python::tuple tup = boost::python::extract<boost::python::tuple>(src);
    Converter<boost::python::tuple, glm::vec2>::assign(tup, dst);
  };
};

template<typename T>
struct Converter<boost::python::list, std::vector<T>>
{
  static void assign(const boost::python::list& src, std::vector<T>& dst)
  {
    size_t count = boost::python::len(src);
    dst.resize(count);
    for (size_t i = 0; i < count; i++) {
      if constexpr (IsInstance<std::vector, T>::value) {  // Nested vector.
        const boost::python::list& lst =
          boost::python::extract<boost::python::list>(src[i]);
        Converter<boost::python::list, T>::assign(lst, dst[i]);
      }
      else {
        Converter<boost::python::api::const_object_item, T>::assign(src[i], dst[i]);
      }
    }
  };
};

template<typename T1, typename T2>
struct Converter<boost::python::tuple, std::pair<T1, T2>>
{
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
  static void assign(const boost::python::api::const_object_item& src,
                     std::pair<T1, T2>&                           dst)
  {
    Converter<boost::python::tuple, std::pair<T1, T2>>::assign(
      boost::python::extract<boost::python::tuple>(src), dst);
  }
};

template<typename T>
struct Converter<data::Tree<T>, boost::python::object>
{
private:
  using DepthT    = data::DepthT;
  using ValIter   = typename std::vector<T>::const_iterator;
  using DepthIter = typename std::vector<DepthT>::const_iterator;

  static void assignLeaf(const T& val, boost::python::object& dst)
  {
    Converter<T, boost::python::object>::assign(val, dst);
  }

  static void copyValues(ValIter&             vbegin,
                         const ValIter&       vend,
                         DepthIter&           dbegin,
                         boost::python::list& dst,
                         DepthT               cdepth = 1)
  {
    if (*dbegin == cdepth) {
      do {
        boost::python::object obj;
        assignLeaf(*vbegin, obj);
        dst.append(obj);
        dbegin++;
        vbegin++;
      } while (*dbegin == 0 && vbegin != vend);
    }
    else if (*dbegin > cdepth) {
      DepthT dcurrent = (*dbegin) - cdepth;
      do {
        boost::python::list lst;
        copyValues(vbegin, vend, dbegin, lst, cdepth + 1);
        dst.append(lst);
        cdepth = 0;
      } while (*dbegin == dcurrent);
    }
  }

public:
  static void assign(const data::Tree<T>& tree, boost::python::object& dst)
  {
    assert(tree.size() != 0 || (tree.size() == 1 && tree.depth(0) == 0));
    if (tree.size() == 0) {
      return;
    }
    if (tree.maxDepth() == 0 && tree.size() > 1) {
      throw std::logic_error("Invalid tree");
    }
    const auto& values = tree.values();
    if (tree.size() == 1 && tree.maxDepth() == 0) {
      assignLeaf(values.front(), dst);
      return;
    }
    const auto& depths = tree.depths();
    assert(values.size() == depths.size());
    auto                vbegin = values.begin();
    auto                vend   = values.end();
    auto                dbegin = depths.begin();
    boost::python::list lst;
    copyValues(vbegin, vend, dbegin, lst);
    assert(vbegin == vend);
    dst = lst;
  };
};

template<typename T>
struct Converter<boost::python::object, data::Tree<T>>
{
private:
  using DepthT = data::DepthT;

  static bool isList(const boost::python::object& obj)
  {
    return boost::python::str(obj.attr("__class__")).find("'list'") != -1;
  }

  static size_t leafCount(const boost::python::object& obj)
  {
    if (isList(obj)) {
      boost::python::list lst    = boost::python::extract<boost::python::list>(obj);
      size_t              length = boost::python::len(lst);
      size_t              total  = 0;
      for (size_t i = 0; i < length; i++) {
        total += leafCount(lst[i]);
      }
      return total;
    }
    else {
      return 1;
    }
  }

  static void assignInternal(const boost::python::object& src,
                             std::vector<T>&              vals,
                             std::vector<DepthT>&         depths,
                             DepthT                       depth = 0)
  {
    if (isList(src)) {
      boost::python::list lst    = boost::python::extract<boost::python::list>(src);
      size_t              length = boost::python::len(lst);
      for (size_t i = 0; i < length; i++) {
        assignInternal(lst[i], vals, depths, i == 0 ? depth + 1 : 0);
      }
    }
    else {
      T val;
      Converter<boost::python::object, T>::assign(src, val);
      vals.push_back(std::move(val));
      depths.push_back(depth);
    }
  }

public:
  static void assign(const boost::python::object& src, data::Tree<T>& dst)
  {
    dst.clear();
    dst.reserve(leafCount(src));
    assignInternal(src, dst.values(), dst.depths());
  }
};

}  // namespace func
}  // namespace gal
