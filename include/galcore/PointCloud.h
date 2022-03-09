#pragma once
#include <atomic>
#include <vector>

#include <tbb/tbb.h>

#include <galcore/Box.h>
#include <galcore/Serialization.h>
#include <glm/glm.hpp>

namespace gal {
class PointCloud : public std::vector<glm::vec3>
{
public:
  PointCloud() = default;
  explicit PointCloud(const std::vector<glm::vec3>&);
  explicit PointCloud(const std::vector<glm::vec2>& pts2d);

  Box3 bounds() const;
};

template<>
struct Serial<PointCloud> : public std::true_type
{
  static PointCloud deserialize(Bytes& bytes)
  {
    PointCloud cloud;
    uint64_t   npts = 0;
    bytes >> npts;
    cloud.resize(npts);
    for (auto& pt : cloud) {
      bytes >> pt;
    }
    return cloud;
  }
  static Bytes serialize(const PointCloud& cloud)
  {
    Bytes dst;
    dst << uint64_t(cloud.size());
    for (const auto& pt : cloud) {
      dst << pt;
    }
    return dst;
  }
};

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
