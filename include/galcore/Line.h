#pragma once

#include <glm/glm.hpp>

#include <galcore/Box.h>
#include <galcore/Serialization.h>

namespace gal {

struct Line2d
{
  glm::vec2 mStart;
  glm::vec2 mEnd;

  Box2 bounds() const;
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

}  // namespace gal