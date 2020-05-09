#pragma once
#include "base.h"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
namespace bgm = boost::geometry::model;
constexpr unsigned int RTREE_NUM_ELEMENTS_PER_NODE = 16;

template <class boost_point_t, typename vec_t, typename box_t>
class rtree
{
public:
    class iterator;
    typedef boost_point_t point_type;
    typedef bgm::box<boost_point_t> box_type;
    typedef std::pair<box_type, size_t> item_type;
    typedef bgi::rtree<item_type, bgi::quadratic<RTREE_NUM_ELEMENTS_PER_NODE>> boost_tree_type;

    void insert(const box_t& b, size_t i)
    {
        m_tree.insert(std::make_pair(to_boost(b), i));
    };

    template <typename size_t_iter>
    void query_box_intersects(const box_t& b, size_t_iter inserter)
    {
        query(bgi::intersects(to_boost(b), inserter));
    };

    template <typename size_t_iter>
    void query_by_distance(const vec_t& pt, double distance)
    {
        point_type center = to_boost(pt);
        query(
            bgi::satisfies([=](const item_type& item) {
                return bg::distance(center, item.first) < distance;
            }));
    };

    iterator& begin() const
    {
        return iterator(m_tree.begin());
    };
    iterator& end() const
    {
        return iterator(m_tree.end());
    };

private:
    boost_tree_type m_tree;

    template <typename predicate_type, typename size_t_iter>
    void query(predicate_type pred, size_t_iter inserter)
    {
        for (auto i = m_tree.qbegin(pred); i = m_tree.qend(); i++)
        {
            *(++inserter) = i->second;
        }
    }

    // Because we created the specializations of these templates, we don't need definitions here.
    static boost_point_t to_boost(const vec_t&);
    static vec_t from_boost(const boost_point_t&);

    static box_type to_boost(const box_t& b)
    {
        return box_type(to_boost(b.min), to_boost(b.max));
    };
    static box_t from_boost(const box_type& b)
    {
        return box_t(from_boost(b.min_corner()), from_boost(b.max_corner()));
    };
};

template class rtree<bgm::point<double, 2, bg::cs::cartesian>, vec2, box2>;
typedef rtree<bgm::point<double, 2, bg::cs::cartesian>, vec2, box2> rtree2d;
template class rtree<bgm::point<double, 3, bg::cs::cartesian>, vec3, box3>;
typedef rtree<bgm::point<double, 3, bg::cs::cartesian>, vec3, box3> rtree3d;

template <>
rtree2d::point_type rtree2d::to_boost(const vec2&);

template <>
vec2 rtree2d::from_boost(const point_type&);

template <>
rtree3d::point_type rtree3d::to_boost(const vec3&);

template <>
vec3 rtree3d::from_boost(const point_type&);

// The iterator class declaration.
template <class boost_point_t, typename vec_t, typename box_t>
class rtree<boost_point_t, vec_t, box_t>::iterator
{
private:
    typedef rtree<boost_point_t, vec_t, box_t>::point_type point_type;
    typedef rtree<boost_point_t, vec_t, box_t>::box_type box_type;
    typedef rtree<boost_point_t, vec_t, box_t>::item_type item_type;
    typedef rtree<boost_point_t, vec_t, box_t>::boost_tree_type boost_tree_type;
    typedef bgi::detail::rtree::iterators::iterator <
        item_type,
        bgi::quadratic<RTREE_NUM_ELEMENTS_PER_NODE>,
        bgi::indexable<item_type>,
        box_type,
        boost::container::new_allocator<item_type>
    > boost_iterator_type;
    typedef rtree<boost_point_t, vec_t, box_t> rtree_type;

    boost_iterator_type m_iter;

public:
    iterator(const boost_iterator_type& iter)
        :m_iter(iter)
    {
    };
    iterator& operator++()
    {
        m_iter++;
    };
    iterator& operator++(int)
    {
        ++m_iter;
    };
    std::pair<box_t, size_t>& operator*()
    {
        return std::make_pair<box_t, size_t>(rtree_type::from_boost(m_iter->first), m_iter->second);
    };
    bool operator==(const iterator& other)
    {
        return m_iter == other.m_iter;
    }
    bool operator!=(const iterator& other)
    {
        return m_iter != other.m_iter;
    }
};