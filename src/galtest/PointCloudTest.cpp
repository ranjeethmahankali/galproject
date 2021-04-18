#include <gtest/gtest.h>
#include <galcore/PointCloud.h>
#include <galcore/Box.h>

using namespace gal;

TEST(PointCloud, Serialization) {
    Box3 b(glm::vec3{-1.f, -1.f, -1.f}, glm::vec3{1.f, 1.f, 1.f});
    PointCloud cloud1;
    static constexpr size_t npts = 1000;
    cloud1.reserve(npts);
    b.randomPoints(npts, std::back_inserter(cloud1));

    ASSERT_EQ(npts, cloud1.size());

    auto bytes = Serial<PointCloud>::serialize(cloud1);
    auto cloud2 = Serial<PointCloud>::deserialize(bytes);

    ASSERT_EQ(npts, cloud2.size());
    ASSERT_EQ(cloud1, cloud2);
};