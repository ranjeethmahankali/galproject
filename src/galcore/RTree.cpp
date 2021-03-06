#include "galcore/RTree.h"

template <> RTree2d::PointType RTree2d::toBoost(const vec2 &v) {
  return PointType(v.x, v.y);
}

template <> vec2 RTree2d::fromBoost(const PointType &p) {
  return vec2(p.get<0>(), p.get<1>());
}

template <> RTree3d::PointType RTree3d::toBoost(const vec3 &v) {
  return PointType(v.x, v.y, v.z);
}

template <> vec3 RTree3d::fromBoost(const PointType &p) {
  return vec3(p.get<0>(), p.get<1>(), p.get<2>());
}