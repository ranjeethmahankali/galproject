#pragma once

#include <span>

#include <galcore/Serialization.h>
#include <galcore/Util.h>
namespace gal {

template<int Dim>
struct Box
{
  using VecT = glm::vec<Dim, float>;
  VecT min;
  VecT max;

  Box();
  Box(const VecT& min, const VecT& max);
  explicit Box(std::span<VecT> points);
  explicit Box(const VecT& pt);

  template<int Dim2>
  explicit Box(const Box<Dim2>& b)
      : min(0.f)
      , max(0.f)
  {
    for (int i = 0; i < Dim2; ++i) {
      min[i] = b.min[i];
      max[i] = b.max[i];
    }
  }

  template<typename PtIter>
  static Box create(PtIter begin, PtIter end)
  {
    Box b;
    while (begin != end) {
      b.inflate(*(begin++));
    }
    return b;
  }

  VecT diagonal() const { return max - min; }
  void inflate(const VecT& v);
  void inflate(const Box<Dim>& b);
  void inflate(float);
  void deflate(float);
  bool contains(const VecT&) const;
  bool contains(const Box<Dim>&) const;
  bool intersects(const Box<Dim>&) const;
  VecT center() const;
  /**
   * @brief In 2d, it represents the area, in 3d it's the volume.
   * It is the generalized higher dimensional volume.
   * @param
   * @return float
   */
  float measure() const;
  bool  valid() const;

  template<typename DstIter>
  void randomPoints(size_t n, DstIter dst) const
  {
    utils::random(min, max, n, dst);
  }

  VecT eval(const VecT& v) const;

  static Box init(const VecT& min, const VecT& max);
};

template<int Dim>
Box<Dim>::Box()
    : min(FLT_MAX)
    , max(-FLT_MAX)
{}

template<int Dim>
Box<Dim>::Box(const VecT& min, const VecT& max)
    : Box()
{
  inflate(min);
  inflate(max);
}

template<int Dim>
Box<Dim>::Box(const VecT& pt)
    : min(pt)
    , max(pt)
{}

template<int Dim>
Box<Dim>::Box(std::span<VecT> pts)
{
  for (const VecT& p : pts) {
    inflate(p);
  }
}

template<int Dim>
void Box<Dim>::inflate(const VecT& v)
{
  min = glm::min(v, min);
  max = glm::max(v, max);
}

template<int Dim>
void Box<Dim>::inflate(const Box<Dim>& b)
{
  inflate(b.min);
  inflate(b.max);
}

template<int Dim>
void Box<Dim>::inflate(float d)
{
  VecT v(d);
  min -= v;
  max += v;
}

template<int Dim>
void Box<Dim>::deflate(float d)
{
  inflate(-d);
}

template<int Dim>
bool Box<Dim>::contains(const VecT& v) const
{
  for (int i = 0; i < Dim; ++i) {
    if (v[i] < min[i] || v[i] > max[i]) {
      return false;
    }
  }
  return true;
}

template<int Dim>
bool Box<Dim>::contains(const Box<Dim>& b) const
{
  return contains(b.min) && contains(b.max);
}

template<int Dim>
bool Box<Dim>::intersects(const Box<Dim>& b) const
{
  return contains(b.min) || contains(b.max);
}

template<int Dim>
typename Box<Dim>::VecT Box<Dim>::center() const
{
  return (min + max) * 0.5f;
}

template<int Dim>
float Box<Dim>::measure() const
{
  VecT  d      = diagonal();
  float result = 1.f;
  for (int i = 0; i < Dim; ++i) {
    result *= d[i];
  }
  return result;
}

template<int Dim>
bool Box<Dim>::valid() const
{
  VecT d = diagonal();
  for (int i = 0; i < Dim; ++i) {
    if (d[i] < 0.f) {
      return false;
    }
  }
  return true;
}

template<int Dim>
typename Box<Dim>::VecT Box<Dim>::eval(const VecT& v) const
{
  VecT result = diagonal();
  for (int i = 0; i < Dim; ++i) {
    result[i] = min[i] + v[i] * result[i];
  }
  return result;
}

template<int Dim>
Box<Dim> Box<Dim>::init(const VecT& min, const VecT& max)
{
  Box b;
  b.min = min;
  b.max = max;
  return b;
}

template<int Dim>
struct Serial<Box<Dim>> : public std::true_type
{
  static Box<Dim> deserialize(Bytes& bytes)
  {
    Box<Dim> box;
    bytes >> box.min >> box.max;
    return box;
  }

  static Bytes serialize(const Box<Dim>& box)
  {
    Bytes bytes;
    bytes << box.min << box.max;
    return bytes;
  }
};

extern template struct Box<2>;
extern template struct Box<3>;

using Box2 = Box<2>;
using Box3 = Box<3>;

}  // namespace gal
