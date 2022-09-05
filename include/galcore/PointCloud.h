#pragma once
#include <atomic>
#include <span>
#include <vector>

#include <tbb/tbb.h>

#include <galcore/Box.h>
#include <galcore/Serialization.h>
#include <glm/glm.hpp>

namespace gal {

template<int NDim>
class PointCloud : public std::vector<glm::vec<NDim, float>>
{
public:
  PointCloud() = default;
  explicit PointCloud(std::span<const glm::vec<NDim, float>> pts)
      : std::vector<glm::vec<NDim, float>>(pts)
  {}

  template<int Dim2>
  explicit PointCloud(std::span<const glm::vec<Dim2, float>> pts)
      : std::vector<glm::vec<NDim, float>>(pts.size(), glm::vec<NDim, float>(0.f))
  {
    for (size_t i = 0; i < pts.size(); ++i) {
      for (int ci = 0; ci < Dim2; ++ci) {
        this->at(i)[ci] = pts[i][ci];
      }
    }
  }

  Box3 bounds() const { return Box3(*this); }
};

extern template class PointCloud<2>;
extern template class PointCloud<3>;

template<int NDim>
struct Serial<PointCloud<NDim>> : public std::true_type
{
  static PointCloud<NDim> deserialize(Bytes& bytes)
  {
    PointCloud<NDim> cloud;
    uint64_t         npts = 0;
    bytes >> npts;
    cloud.resize(npts);
    for (auto& pt : cloud) {
      bytes >> pt;
    }
    return cloud;
  }
  static Bytes serialize(const PointCloud<NDim>& cloud)
  {
    Bytes dst;
    dst << uint64_t(cloud.size());
    for (const auto& pt : cloud) {
      dst << pt;
    }
    return dst;
  }
};

/**
 * @brief Computes the k-means clustering for the given range of points.
 *
 * @tparam TPt The type of point. Must be either glm::vec3 or glm::vec2
 * @tparam TPtIter The iterator type.
 * @tparam IdxOutIter Output iterator type that can accept size_t
 * @param begin Iterator to the first point.
 * @param end Iterator past the last point.
 * @param nClusters Number of clusters.
 * @param idxOut Output iterator for the index of the point cloud.
 */
template<typename TPt, typename TPtIter, typename IdxOutIter>
void kMeansClusters(TPtIter begin, TPtIter end, size_t nClusters, IdxOutIter idxOut)
{
  static_assert(std::is_same_v<TPt, glm::vec3> || std::is_same_v<TPt, glm::vec2>,
                "Unsupported point type.");
  static constexpr TPt sZeroVec = TPt(0.f);
  using BoxType = std::conditional_t<std::is_same_v<TPt, glm::vec3>, Box3, Box2>;

  size_t              nPts = std::distance(begin, end);
  auto                box  = BoxType::template create<TPtIter>(begin, end);
  std::vector<TPt>    centers(nClusters);
  std::vector<size_t> counts(nClusters);
  box.randomPoints(nClusters, centers.begin());

  std::vector<std::pair<float, size_t>> mapping(nPts, std::make_pair(FLT_MAX, SIZE_MAX));

  std::atomic_bool keepGoing = true;
  while (keepGoing) {
    keepGoing = false;
    tbb::parallel_for(
      size_t(0), nPts, [&centers, &mapping, &keepGoing, &begin](size_t i) {
        auto& map            = mapping.at(i);
        map.first            = FLT_MAX;
        size_t      minIndex = SIZE_MAX;
        const auto& pt       = *(begin + i);
        for (size_t ci = 0; ci < centers.size(); ci++) {
          float d2 = glm::distance2(pt, centers[ci]);
          if (d2 < map.first) {
            map.first = d2;
            minIndex  = ci;
          }
        }
        keepGoing  = keepGoing || map.second != minIndex;
        map.second = minIndex;
      });

    if (keepGoing) {
      std::fill(centers.begin(), centers.end(), sZeroVec);
      std::fill(counts.begin(), counts.end(), size_t(0));
      for (size_t i = 0; i < nPts; i++) {
        const auto& map = mapping.at(i);
        centers[map.second] += *(begin + i);
        counts[map.second]++;
      }
      for (size_t i = 0; i < centers.size(); i++) {
        centers[i] /= float(counts[i]);
      }
    }
  }

  std::transform(
    mapping.begin(), mapping.end(), idxOut, [](const std::pair<float, size_t>& m) {
      return m.second;
    });
}

}  // namespace gal
