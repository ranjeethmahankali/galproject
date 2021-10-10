#include <gtest/gtest.h>

#include <galcore/Timer.h>
#include <galcore/Util.h>
#include <galfunc/Data.h>

using namespace gal::func::data;

static DataTree<int> testTree()
{
  DataTree<int> tree;
  auto          outview = OutputView<int>(tree);
  for (size_t i = 0; i < 32; i++) {
    auto d = DepthT(std::min(size_t(5), size_t(gal::utils::bitscanForward(i))));
    outview.push_back(d, i);
  }

  return tree;
}

TEST(Data, CreateTree)
{
  auto tree = testTree();
  for (size_t i = 0; i < tree.size(); i++) {
    ASSERT_EQ(tree.depth(i),
              DepthT(std::min(size_t(5), size_t(gal::utils::bitscanForward(i)))));
  }
}

TEST(Data, ViewIterators)
{
  auto tree = testTree();
  int  i    = 0;
  auto v5   = InputView<int, 5>(tree);
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
  DataTree<glm::vec3>     tree1;
  auto                    tree = OutputView(tree1);
  tree.reserve(nPoints);
  gal::utils::random(glm::vec3 {-1.f, -1.f, -1.f},
                     glm::vec3 {1.f, 1.f, 1.f},
                     nPoints,
                     std::back_inserter(tree));

  for (size_t i = 0; i < tree1.size(); i++) {
    tree1.depth(i) = (i % nL2Size) == 0 ? 2 : (i % nL1Size) == 0 ? 1 : 0;
  }

  InputView<glm::vec3, 3> view(tree1);

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
    for (size_t i = 0; i < tree1.size(); i += nL1Size) {
      sum2 += tree1.value(i);
    }
  }

  ASSERT_EQ(sum1, sum2);

  std::cout << "Access time: " << accessTime.count() << " ns\n";
  std::cout << "Control time: " << controlTime.count() << " ns\n";
  std::cout << "Ratio: " << float(accessTime.count()) / float(controlTime.count())
            << std::endl;
}
