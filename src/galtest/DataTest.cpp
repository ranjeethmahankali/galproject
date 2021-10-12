#include <gtest/gtest.h>

#include <galcore/Timer.h>
#include <galcore/Util.h>
#include <galfunc/Data.h>
#include <glm/fwd.hpp>

using namespace gal::func::data;

static Tree<int> testTree()
{
  Tree<int> tree;
  tree.reserve(32);
  for (size_t i = 0; i < 32; i++) {
    auto d = DepthT(std::min(size_t(5), size_t(gal::utils::bitscanForward(i))));
    tree.push_back(d, i);
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
  auto v5   = ReadView<int, 5>(tree);
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
  Tree<glm::vec3>         tree;
  {
    auto v3 = WriteView<glm::vec3, 3>(tree);
    v3.reserve(nPoints);
    for (size_t i = 0; i < nPoints; i += nL2Size) {
      auto v2 = v3.child();
      for (size_t j = 0; j < nL2Size; j += nL1Size) {
        auto v1 = v2.child();
        gal::utils::random(glm::vec3 {-1.f, -1.f, -1.f},
                           glm::vec3 {1.f, 1.f, 1.f},
                           nL1Size,
                           std::back_inserter(v1));
      }
    }
  }

  ASSERT_EQ(tree.size(), nPoints);
  ASSERT_EQ(tree.maxDepth(), 3);

  ReadView<glm::vec3, 3> view(tree);

  std::chrono::nanoseconds accessTime;
  glm::vec3                sum1 {0.f, 0.f, 0.f};
  glm::vec3                sum2 {0.f, 0.f, 0.f};
  {
    gal::Timer timer("read-test", &accessTime);
    for (auto l2 : view) {
      for (auto lst : l2)
        sum1 += lst[0];
    }
  }

  std::chrono::nanoseconds controlTime;
  {
    gal::Timer timer("control", &controlTime);
    for (size_t i = 0; i < tree.size(); i += nL1Size) {
      sum2 += tree.value(i);
    }
  }

  ASSERT_EQ(sum1, sum2);

  std::cout << "Access time: " << accessTime.count() << " ns\n";
  std::cout << "Control time: " << controlTime.count() << " ns\n";
  std::cout << "Ratio: " << float(accessTime.count()) / float(controlTime.count())
            << std::endl;
}

TEST(Data, WritePerformance)
{
  static constexpr size_t nPoints = 100000;
  static constexpr size_t nL2Size = 10000;
  static constexpr size_t nL1Size = 1000;

  std::vector<glm::vec3> points(nPoints);
  gal::utils::random(glm::vec3 {-1.f, -1.f, -1.f},
                     glm::vec3 {1.f, 1.f, 1.f},
                     points.size(),
                     points.begin());

  auto                     src = points.begin();
  std::chrono::nanoseconds writeTime;
  {
    Tree<glm::vec3> tree;
    {
      gal::Timer timer("read-test", &writeTime);
      auto       v3 = WriteView<glm::vec3, 3>(tree);
      v3.reserve(nPoints);
      for (size_t i = 0; i < nPoints; i += nL2Size) {
        auto v2 = v3.child();
        for (size_t j = 0; j < nL2Size; j += nL1Size) {
          auto v1 = v2.child();
          std::copy_n(src, nL1Size, std::back_inserter(v1));
          src += nL1Size;
        }
      }
    }
    ASSERT_EQ(tree.values(), points);
  }

  std::chrono::nanoseconds controlTime;
  {
    Tree<glm::vec3> tree;
    {
      gal::Timer timer("control", &controlTime);
      auto&      dstVec = tree.values();
      dstVec.reserve(points.size());
      std::copy(points.begin(), points.end(), std::back_inserter(dstVec));
    }
    ASSERT_EQ(tree.values(), points);
  }

  std::cout << "Write time: " << writeTime.count() << " ns\n";
  std::cout << "Control time: " << controlTime.count() << " ns\n";
  std::cout << "Ratio: " << float(writeTime.count()) / float(controlTime.count())
            << std::endl;
}
