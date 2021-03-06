#include "galcore/RTree.h"

template <> RTree2d::PointType RTree2d::toBoost(const glm::vec2 &v) {
  return PointType(v.x, v.y);
}

template <> glm::vec2 RTree2d::fromBoost(const PointType &p) {
  return glm::vec2(p.get<0>(), p.get<1>());
}

template <> RTree3d::PointType RTree3d::toBoost(const glm::vec3 &v) {
  return PointType(v.x, v.y, v.z);
}

template <> glm::vec3 RTree3d::fromBoost(const PointType &p) {
  return glm::vec3(p.get<0>(), p.get<1>(), p.get<2>());
}