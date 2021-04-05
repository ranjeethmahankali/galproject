#pragma once
#include <galcore/Box.h>
#include <galcore/Util.h>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <glm/glm.hpp>

constexpr unsigned int RTREE_NUM_ELEMENTS_PER_NODE = 16;
namespace bg                                       = boost::geometry;
namespace bgi                                      = boost::geometry::index;
namespace bgm                                      = boost::geometry::model;
namespace rtree                                    = bgi::detail::rtree;

template<typename Predicate,
         typename Value,
         typename Options,
         typename Box,
         typename Allocators>
struct BFSQuery : public rtree::visitor<Value,
                                        typename Options::parameters_type,
                                        Box,
                                        Allocators,
                                        typename Options::node_tag,
                                        true>::type
{
  typedef typename rtree::internal_node<Value,
                                        typename Options::parameters_type,
                                        Box,
                                        Allocators,
                                        typename Options::node_tag>::type internal_node;
  typedef typename rtree::leaf<Value,
                               typename Options::parameters_type,
                               Box,
                               Allocators,
                               typename Options::node_tag>::type          leaf;

  inline BFSQuery(Predicate const& p)
      : pr(p)
  {}

  inline void operator()(internal_node const& n)
  {
    for (auto&& [bounds, node] : rtree::elements(n))
      if (pr(bounds))
        rtree::apply_visitor(*this, *node);
  }

  inline void operator()(leaf const& n)
  {
    for (auto& item : rtree::elements(n))
      if (pr(item.first))
        results.insert(&item);
  }

  Predicate const& pr;

  std::set<Value const*> results;
};

template<typename TreeT, typename PredFn, typename Fn>
void doBfsQuery(const TreeT& tree, PredFn pred, Fn action)
{
  using V = rtree::utilities::view<TreeT>;
  V av(tree);

  BFSQuery<PredFn,
           typename V::value_type,
           typename V::options_type,
           typename V::box_type,
           typename V::allocators_type>
    vis(pred);

  av.apply_visitor(vis);
  for (auto* hit : vis.results)
    action(*hit);
};

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
    PointType  center = toBoost(pt);
    const auto pred   = [&center, distance](const BoxType& bounds) {
      return bg::distance(center, bounds) < distance;
    };
    const auto action = [&inserter](const ItemType& item) {
      *(inserter++) = item.second;
    };
    doBfsQuery<BoostTreeType, decltype(pred), decltype(action)>(mTree, pred, action);
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

public:
  void clear() { mTree.clear(); };
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