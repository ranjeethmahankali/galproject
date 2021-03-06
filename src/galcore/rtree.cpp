#include "galcore/rtree.h"

template <> rtree2d::point_type rtree2d::to_boost(const vec2 &v) {
  return point_type(v.x, v.y);
}

template <> vec2 rtree2d::from_boost(const point_type &p) {
  return vec2(p.get<0>(), p.get<1>());
}

template <> rtree3d::point_type rtree3d::to_boost(const vec3 &v) {
  return point_type(v.x, v.y, v.z);
}

template <> vec3 rtree3d::from_boost(const point_type &p) {
  return vec3(p.get<0>(), p.get<1>(), p.get<2>());
}