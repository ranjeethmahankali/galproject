#pragma once

#include <filesystem>
#include <type_traits>

#include <pybind11/pybind11.h>
#include <glm/glm.hpp>

#include <galfunc/Data.h>

namespace py = pybind11;

namespace gal {
namespace func {

template<typename T1, typename T2>
struct Converter
{
  T2 operator()(const T1& src) const
  {
    if constexpr (std::is_same_v<T1, T2>) {
      return src;
    }
    else {
      return T2(src);
    }
  }
};

template<typename T>
struct Converter<py::object, T>
{
  static void assign(const py::object& obj, T& dst) { dst = obj.cast<T>(); }
};

template<int N, typename T, glm::qualifier Q>
struct Converter<glm::vec<N, T, Q>, py::object>
{
  static void assign(const glm::vec<N, T, Q>& src, py::object& dst)
  {
    auto tup = py::tuple(N);
    for (int i = 0; i < N; ++i) {
      tup[i] = src[i];
    }
    dst = tup;
  }
};

template<int N, typename T, glm::qualifier Q>
struct Converter<py::object, glm::vec<N, T, Q>>
{
  static void assign(const py::object& src, glm::vec<N, T, Q>& dst)
  {
    py::tuple tup = py::cast<py::tuple>(src);
    for (int i = 0; i < N; ++i) {
      dst[i] = py::cast<float>(tup[i]);
    }
  };
};

template<int N, typename T, glm::qualifier Q>
struct Converter<py::tuple, glm::vec<N, T, Q>>
{
  static void assign(const py::tuple& src, glm::vec<N, T, Q>& dst)
  {
    for (int i = 0; i < N; ++i) {
      dst[i] = py::cast<float>(src[i]);
    }
  }
};

template<typename T>
struct Converter<py::list, std::vector<T>>
{
  static void assign(const py::list& src, std::vector<T>& dst)
  {
    dst.resize(py::len(src));
    for (size_t i = 0; i < dst.size(); i++) {
      if constexpr (IsInstance<std::vector, T>::value) {  // Nested vector.
        const py::list& lst = py::cast<py::list>(src[i]);
        Converter<py::list, T>::assign(lst, dst[i]);
      }
      else {
        Converter<py::object, T>::assign(src[i], dst[i]);
      }
    }
  };
};

template<typename T>
struct Converter<std::vector<T>, py::list>
{
  static void assign(const std::vector<T>& src, py::list& dst)
  {
    for (size_t i = 0; i < src.size(); i++) {
      if constexpr (IsInstance<std::vector, T>::value) {  // Nested vector.
        py::list lst;
        Converter<T, py::list>::assign(src[i], lst);
        dst.append(lst);
      }
      else {
        py::object obj;
        Converter<T, py::object>::assign(src[i], obj);
        dst.append(obj);
      }
    }
  }
};

template<typename T1, typename T2>
struct Converter<py::tuple, std::pair<T1, T2>>
{
  static void assign(const py::tuple& src, std::pair<T1, T2>& dst)
  {
    Converter<py::object, T1>::assign(src[0], std::get<0>(dst));
    Converter<py::object, T2>::assign(src[1], std::get<1>(dst));
  }
};

template<typename T1, typename T2>
struct Converter<py::object, std::pair<T1, T2>>
{
  static void assign(const py::object& src, std::pair<T1, T2>& dst)
  {
    Converter<py::tuple, std::pair<T1, T2>>::assign(py::cast<py::tuple>(src), dst);
  }
};

template<>
struct Converter<py::object, Bool>
{
  static void assign(const py::object& src, Bool& dst)
  {
    dst = Bool(py::cast<bool>(src));
  }
};

template<>
struct Converter<Bool, py::object>
{
  static void assign(const Bool& src, py::object& dst) { dst = py::bool_(bool(src)); }
};

template<>
struct Converter<py::object, std::filesystem::path>
{
  static void assign(const py::object& src, std::filesystem::path& dst)
  {
    dst = std::string(py::cast<std::string>(src));
  }
};

template<typename T>
struct Converter<data::Tree<T>, py::object>
{
private:
  using DepthT    = data::DepthT;
  using ValueType = typename data::Tree<T>::ValueType;
  using ValIter   = typename data::Tree<T>::InternalStorageT::const_iterator;
  using DepthIter = typename std::vector<DepthT>::const_iterator;
  static constexpr bool IsSharedPtr =
    std::is_same_v<std::shared_ptr<T>, std::remove_const_t<ValueType>>;

  static void assignLeaf(const ValueType& val, py::object& dst)
  {
    if constexpr (IsSharedPtr) {
      Converter<T, py::object>::assign(*val, dst);
    }
    else {
      Converter<T, py::object>::assign(val, dst);
    }
  }

  static void copyValues(ValIter&       vbegin,
                         const ValIter& vend,
                         DepthIter&     dbegin,
                         py::list&      dst,
                         DepthT         cdepth = 1)
  {
    if (*dbegin == cdepth) {
      do {
        py::object obj;
        assignLeaf(*vbegin, obj);
        dst.append(obj);
        ++dbegin;
        ++vbegin;
      } while (*dbegin == 0 && vbegin != vend);
    }
    else if (*dbegin > cdepth) {
      DepthT dcurrent = (*dbegin) - cdepth;
      do {
        py::list lst;
        copyValues(vbegin, vend, dbegin, lst, cdepth + 1);
        dst.append(lst);
        cdepth = 0;
      } while (*dbegin == dcurrent);
    }
  }

public:
  static void assign(const data::Tree<T>& tree, py::object& dst)
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
    auto     vbegin = values.begin();
    auto     vend   = values.end();
    auto     dbegin = depths.begin();
    py::list lst;
    copyValues(vbegin, vend, dbegin, lst);
    assert(vbegin == vend);
    dst = lst;
  };
};

template<typename T>
struct Converter<py::object, data::Tree<T>>
{
private:
  using DepthT    = data::DepthT;
  using ValueType = typename data::Tree<T>::ValueType;
  using ValIter   = typename data::Tree<T>::InternalStorageT::const_iterator;
  using DepthIter = typename std::vector<DepthT>::const_iterator;
  static constexpr bool IsSharedPtr =
    std::is_same_v<std::shared_ptr<T>, std::remove_const_t<ValueType>>;

  static bool isList(const py::object& obj)
  {
    return py::str(obj.attr("__class__")).contains("'list'");
  }

  static size_t leafCount(const py::object& obj)
  {
    if (isList(obj)) {
      py::list lst    = py::cast<py::list>(obj);
      size_t   length = py::len(lst);
      size_t   total  = 0;
      for (size_t i = 0; i < length; i++) {
        total += leafCount(lst[i]);
      }
      return total;
    }
    else {
      return 1;
    }
  }

  static void assignInternal(const py::object&       src,
                             std::vector<ValueType>& vals,
                             std::vector<DepthT>&    depths,
                             DepthT                  depth = 0)
  {
    if (isList(src)) {
      py::list lst    = py::cast<py::list>(src);
      size_t   length = py::len(lst);
      for (size_t i = 0; i < length; i++) {
        assignInternal(lst[i], vals, depths, i == 0 ? depth + 1 : 0);
      }
    }
    else {
      ValueType val;
      if constexpr (IsSharedPtr) {
        val = std::make_shared<T>();
        Converter<py::object, T>::assign(src, *val);
      }
      else {
        Converter<py::object, T>::assign(src, val);
      }
      vals.push_back(std::move(val));
      depths.push_back(depth);
    }
  }

public:
  static void assign(const py::object& src, data::Tree<T>& dst)
  {
    dst.clear();
    dst.reserve(leafCount(src));
    assignInternal(src, dst.values(), dst.depths());
  }
};

}  // namespace func
}  // namespace gal
