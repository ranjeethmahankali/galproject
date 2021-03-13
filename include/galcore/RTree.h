#pragma once
#include <galcore/Box.h>
#include <galcore/Util.h>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>

namespace bg                                       = boost::geometry;
namespace bgi                                      = boost::geometry::index;
namespace bgm                                      = boost::geometry::model;
constexpr unsigned int RTREE_NUM_ELEMENTS_PER_NODE = 16;

template<class BoostPointT, typename VecT, typename BoxT>
class RTree
{
public:
  typedef BoostPointT                                                       PointType;
  typedef bgm::box<BoostPointT>                                             BoxType;
  typedef std::pair<BoxType, size_t>                                        ItemType;
  typedef bgi::rtree<ItemType, bgi::quadratic<RTREE_NUM_ELEMENTS_PER_NODE>> BoostTreeType;

  void insert(const BoxT& b, size_t i) { mTree.insert(std::make_pair(toBoost(b), i)); };

  template<typename SizeTIter>
  void queryBoxIntersects(const BoxT& b, SizeTIter inserter) const
  {
    query(bgi::intersects(toBoost(b)), inserter);
  };

  template<typename SizeTIter>
  void queryByDistance(const VecT& pt, float distance, SizeTIter inserter) const
  {
    PointType center = toBoost(pt);
    query(bgi::satisfies([=](const ItemType& item) {
            return bg::distance(center, item.first) < distance;
          }),
          inserter);
  };

  template<typename SizeTIter>
  void queryNearestN(const VecT& pt, size_t numResults, SizeTIter inserter) const
  {
    query(bgi::nearest(toBoost(pt), (unsigned int)numResults), inserter);
  };

private:
  BoostTreeType mTree;

  template<typename predicate_type, typename SizeTIter>
  void query(predicate_type pred, SizeTIter inserter) const
  {
    for (auto i = mTree.qbegin(pred); i != mTree.qend(); i++) {
      *(inserter++) = i->second;
    }
  }

  // Because we created the specializations of these templates, we don't need
  // definitions here.
  static BoostPointT toBoost(const VecT&);
  static VecT        fromBoost(const BoostPointT&);

  static BoxType toBoost(const BoxT& b)
  {
    return BoxType(toBoost(b.min), toBoost(b.max));
  };
  static BoxT fromBoost(const BoxType& b)
  {
    return BoxT(fromBoost(b.min_corner()), fromBoost(b.max_corner()));
  };
};

template class RTree<bgm::point<float, 2, bg::cs::cartesian>, glm::vec2, gal::Box2>;
typedef RTree<bgm::point<float, 2, bg::cs::cartesian>, glm::vec2, gal::Box2> RTree2d;
template class RTree<bgm::point<float, 3, bg::cs::cartesian>, glm::vec3, gal::Box3>;
typedef RTree<bgm::point<float, 3, bg::cs::cartesian>, glm::vec3, gal::Box3> RTree3d;

template<>
RTree2d::PointType RTree2d::toBoost(const glm::vec2&);

template<>
glm::vec2 RTree2d::fromBoost(const PointType&);

template<>
RTree3d::PointType RTree3d::toBoost(const glm::vec3&);

template<>
glm::vec3 RTree3d::fromBoost(const PointType&);