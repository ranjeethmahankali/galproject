#include <execution>

#include <galcore/Box.h>
#include <galcore/PointCloud.h>
#include <gtest/gtest.h>

using namespace gal;

TEST(PointCloud, Serialization)
{
  Box3                    b(glm::vec3 {-1.f, -1.f, -1.f}, glm::vec3 {1.f, 1.f, 1.f});
  PointCloud<3>           cloud1;
  static constexpr size_t npts = 1000;
  cloud1.reserve(npts);
  b.randomPoints(npts, std::back_inserter(cloud1));

  ASSERT_EQ(npts, cloud1.size());

  auto bytes  = Serial<PointCloud<3>>::serialize(cloud1);
  auto cloud2 = Serial<PointCloud<3>>::deserialize(bytes);

  ASSERT_EQ(npts, cloud2.size());
  ASSERT_EQ(cloud1, cloud2);
}

TEST(PointCloud, KMeansClusters)
{
  gal::Box3               bounds(glm::vec3(0.f), glm::vec3(10.f));
  static constexpr size_t nPoints   = 100;
  static constexpr size_t nClusters = 5;
  std::vector<glm::vec3>  points(nPoints);
  bounds.randomPoints(points.size(), points.begin());
  std::vector<size_t> indices(points.size());
  gal::kMeansClusters<glm::vec3>(
    points.begin(), points.end(), nClusters, indices.begin());

  for (size_t i : indices) {
    ASSERT_TRUE(i < nClusters);
  }

  for (size_t i = 0; i < nClusters; i++) {
    size_t clusterSize =
      std::count(std::execution::par_unseq, indices.begin(), indices.end(), i);
    ASSERT_TRUE(clusterSize > 0);
  }
}
