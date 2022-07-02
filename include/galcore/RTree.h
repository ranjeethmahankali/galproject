#pragma once
#include <galcore/Box.h>
#include <galcore/Util.h>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <glm/glm.hpp>
#include <utility>

namespace gal {

constexpr unsigned int RTREE_NUM_ELEMENTS_PER_NODE = 16;
namespace bg                                       = boost::geometry;
namespace bgi                                      = boost::geometry::index;
namespace bgm                                      = boost::geometry::model;
namespace rtree                                    = bgi::detail::rtree;

template<typename TSrc, typename TDst>
struct Convert
{
  TDst operator()(const TSrc& v) const { return TDst(v); }
};

template<int N, typename T, glm::qualifier Q>
struct Convert<glm::vec<N, T, Q>, bgm::point<float, size_t(N), bg::cs::cartesian>>
{
  using BoostPointT = bgm::point<float, size_t(N), bg::cs::cartesian>;
  using GlmT        = glm::vec<N, T, Q>;

  template<int... Is>
  inline BoostPointT convert(const GlmT& p, std::integer_sequence<int, Is...>) const
  {
    return BoostPointT(p[Is]...);
  }

public:
  BoostPointT operator()(const GlmT& p) const
  {
    return convert(p, std::make_integer_sequence<int, N> {});
  }
};

template<int N, typename T, glm::qualifier Q>
struct Convert<bgm::point<float, size_t(N), bg::cs::cartesian>, glm::vec<N, T, Q>>
{
  using BoostPointT = bgm::point<float, size_t(N), bg::cs::cartesian>;
  using GlmT        = glm::vec<N, T, Q>;

  template<int... Is>
  inline GlmT convert(const BoostPointT& p, std::integer_sequence<int, Is...>) const
  {
    return GlmT(p.template get<Is>()...);
  }

public:
  GlmT operator()(const BoostPointT& p) const
  {
    return convert(p, std::make_integer_sequence<int, N> {});
  }
};

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

  inline explicit BFSQuery(Predicate const& p)
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
  using PointType     = BoostPointT;
  using BoxType       = bgm::box<BoostPointT>;
  using ItemType      = std::pair<BoxType, int>;
  using BoostTreeType = bgi::rtree<ItemType, bgi::quadratic<RTREE_NUM_ELEMENTS_PER_NODE>>;

  void insert(const BoxT& b, int i) { mTree.insert(std::make_pair(toBoost(b), i)); };

  template<typename IntIter>
  void queryBoxIntersects(const BoxT& b, IntIter inserter) const
  {
    query(bgi::intersects(toBoost(b)), inserter);
  };

  template<typename IntIter>
  void queryByDistance(const VecT& pt, float distance, IntIter inserter) const
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

  template<typename IntIter>
  void queryNearestN(const VecT& pt, size_t numResults, IntIter inserter) const
  {
    query(bgi::nearest(toBoost(pt), (unsigned int)numResults), inserter);
  };

private:
  BoostTreeType mTree;

  template<typename predicate_type, typename IntIter>
  void query(predicate_type pred, IntIter inserter) const
  {
    for (auto i = mTree.qbegin(pred); i != mTree.qend(); i++) {
      *(inserter++) = i->second;
    }
  }

  static BoostPointT toBoost(const VecT& p)
  {
    static Convert<VecT, BoostPointT> sToBoost;
    return sToBoost(p);
  }

  static VecT fromBoost(const BoostPointT& p)
  {
    static Convert<BoostPointT, VecT> sFromBoost;
    return sFromBoost(p);
  }

  static BoxType toBoost(const BoxT& b)
  {
    return BoxType(toBoost(b.min), toBoost(b.max));
  }

  static BoxT fromBoost(const BoxType& b)
  {
    return BoxT(fromBoost(b.min_corner()), fromBoost(b.max_corner()));
  }

public:
  void clear() { mTree.clear(); };
};

using BoostPoint2d = bgm::point<float, 2, bg::cs::cartesian>;
using BoostPoint3d = bgm::point<float, 3, bg::cs::cartesian>;
using RTree2d      = RTree<BoostPoint2d, glm::vec2, gal::Box2>;
using RTree3d      = RTree<BoostPoint3d, glm::vec3, gal::Box3>;

}  // namespace gal
