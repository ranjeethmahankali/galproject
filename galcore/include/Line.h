#pragma once

#include <glm/glm.hpp>

#include <Box.h>
#include <Serialization.h>

namespace gal {

struct Line2d
{
  glm::vec2 mStart = {0.f, 0.f};
  glm::vec2 mEnd   = {0.f, 0.f};

  Box2      bounds() const;
  glm::vec2 vec() const;
};

struct Line3d
{
  glm::vec3 mStart = {0.f, 0.f, 0.f};
  glm::vec3 mEnd   = {0.f, 0.f, 0.f};

  Box3      bounds() const;
  glm::vec3 vec() const;
};

struct Polyline
{
  std::vector<glm::vec3> mPoints;
};

template<>
struct Serial<Line2d> : public std::true_type
{
  static Line2d deserialize(Bytes& bytes)
  {
    Line2d line;
    bytes >> line.mStart >> line.mEnd;
    return line;
  }

  static Bytes serialize(const Line2d& line)
  {
    Bytes bytes;
    bytes << line.mStart << line.mEnd;
    return bytes;
  }
};

template<>
struct Serial<Line3d> : public std::true_type
{
  static Line3d deserialize(Bytes& bytes)
  {
    Line3d line;
    bytes >> line.mStart >> line.mEnd;
    return line;
  }

  static Bytes serialize(const Line3d& line)
  {
    Bytes bytes;
    bytes << line.mStart << line.mEnd;
    return bytes;
  }
};

template<>
struct Serial<Polyline> : public std::true_type
{
  static Polyline deserialize(Bytes& bytes)
  {
    Polyline pline;
    bytes >> pline.mPoints;
    return pline;
  }

  static Bytes serialize(Polyline const& pline)
  {
    Bytes bytes;
    bytes << pline.mPoints;
    return bytes;
  }
};

}  // namespace gal
