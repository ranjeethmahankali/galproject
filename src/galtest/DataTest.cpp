#include <gtest/gtest.h>

#include <galcore/Timer.h>
#include <galcore/Util.h>
#include <galfunc/Data.h>

using namespace gal::func;

static DataTree<int> testTree()
{
  DataTree<int> tree;
  for (size_t i = 0; i < 32; i++) {
    uint32_t d = uint32_t(std::min(size_t(5), size_t(gal::utils::bitscanForward(i))));
    tree.push_back(d, i);
  }

  return tree;
}

TEST(Data, CreateTree)
{
  auto tree = testTree();
  for (size_t i = 0; i < tree.size(); i++) {
    ASSERT_EQ(tree.mValues[i].mDepth,
              uint32_t(std::min(size_t(5), size_t(gal::utils::bitscanForward(i)))));
  }
}

TEST(Data, ViewIterators)
{
  auto tree = testTree();
  int  i    = 0;
  auto v5   = decltype(tree)::ReadView<5>(tree);
  for (auto v4 : v5) {
    for (auto v3 : v4) {
      for (auto v2 : v3) {
        for (auto v1 : v2) {
          for (auto v0 : v1) {
            ASSERT_EQ(v0, i++);
          }
        }
      }
    }
  }

  ASSERT_EQ(tree.size(), size_t(i));
}

TEST(Data, ReadPerformance)
{
  static constexpr size_t nPoints = 100000;
  static constexpr size_t nL2Size = 10000;
  static constexpr size_t nL1Size = 1000;
  DataTree<glm::vec3>     tree;
  tree.reserve(nPoints);
  gal::utils::random(glm::vec3 {-1.f, -1.f, -1.f},
                     glm::vec3 {1.f, 1.f, 1.f},
                     nPoints,
                     std::back_inserter(tree));

  for (size_t i = 0; i < tree.size(); i++) {
    tree.mValues[i].mDepth = (i % nL2Size) == 0 ? 2 : (i % nL1Size) == 0 ? 1 : 0;
  }

  DataTree<glm::vec3>::ReadView<3> view(tree);

  std::chrono::nanoseconds accessTime;
  std::chrono::nanoseconds controlTime;
  glm::vec3                sum1 {0.f, 0.f, 0.f};
  glm::vec3                sum2 {0.f, 0.f, 0.f};
  {
    gal::Timer timer("read-test", &accessTime);
    for (auto l2 : view) {
      for (auto lst : l2)
        sum1 += lst[0];
    }
  }

  {
    gal::Timer timer("control", &controlTime);
    for (size_t i = 0; i < tree.size(); i += nL1Size) {
      sum2 += tree.mValues[i].mValue;
    }
  }

  ASSERT_EQ(sum1, sum2);

  std::cout << "Access time: " << accessTime.count() << " ns\n";
  std::cout << "Control time: " << controlTime.count() << " ns\n";
}
