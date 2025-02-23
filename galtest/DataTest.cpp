#include <catch2/catch_all.hpp>

#include <catch2/catch_test_macros.hpp>
#include <glm/glm.hpp>
#include <numeric>

#include <Data.h>
#include <Functions.h>
#include <TestUtils.h>
#include <Timer.h>
#include <Util.h>

using namespace gal::func::data;

static Tree<int> testTree()
{
  Tree<int> tree;
  tree.reserve(32);
  for (uint64_t i = 0; i < 32; i++) {
    auto d = DepthT(std::min(uint64_t(5), uint64_t(gal::utils::bitscanForward(i))));
    tree.push_back(d, int(i));
  }

  return tree;
}

TEST_CASE("Data - CreateTree", "[tree][create]")  // NOLINT
{
  auto tree = testTree();
  for (uint64_t i = 0; i < tree.size(); i++) {
    REQUIRE(tree.depth(i) ==
            DepthT(std::min(uint64_t(5), uint64_t(gal::utils::bitscanForward(i)))));
  }
}

TEST_CASE("Data - ViewIterators", "[tree][iterators]")  // NOLINT
{
  auto tree = testTree();
  int  i    = 0;
  auto v5   = ReadView<int, 5>(tree);
  for (auto v4 : v5) {
    for (auto v3 : v4) {
      for (auto v2 : v3) {
        for (auto v1 : v2) {
          for (auto v0 : v1) {
            REQUIRE(v0 == i++);
          }
        }
      }
    }
  }

  REQUIRE(tree.size() == size_t(i));

  i       = 0;
  auto v4 = ReadView<int, 4>(tree);
  for (auto v3 : v4) {
    for (auto v2 : v3) {
      for (auto v1 : v2) {
        for (auto v0 : v1) {
          REQUIRE(v0 == i++);
        }
      }
    }
  }

  REQUIRE(i == 16);

  int outer = 0;
  i         = 0;
  auto v3   = ReadView<int, 3>(tree);
  while (outer++ < 4) {
    for (auto v2 : v3) {
      for (auto v1 : v2) {
        for (auto v0 : v1) {
          REQUIRE(v0 == i++);
        }
      }
    }
    REQUIRE(i == 8 * outer);
    if (!v3.tryAdvance()) {
      break;
    }
  }
}

TEST_CASE("Data - ReadPerformance", "[data][perf]")  // NOLINT
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

  REQUIRE(tree.size() == nPoints);
  REQUIRE(tree.maxDepth() == 3);

  ReadView<glm::vec3, 3> view(tree);

  std::chrono::nanoseconds accessTime;
  glm::vec3                sum1;
  glm::vec3                sum2 = {0.f, 0.f, 0.f};
  {
    gal::Timer timer("read-test", &accessTime);
    sum1 = std::accumulate(
      view.begin(), view.end(), glm::vec3(0.f), [](glm::vec3 s2, const auto& v2) {
        return std::accumulate(
          v2.begin(), v2.end(), s2, [](glm::vec3 s1, const auto& v1) {
            return s1 + v1[0];
          });
      });
  }

  std::chrono::nanoseconds controlTime;
  {
    gal::Timer timer("control", &controlTime);
    for (size_t i = 0; i < tree.size(); i += nL1Size) {
      sum2 += tree.value(i);
    }
  }

  REQUIRE(sum1 == sum2);

  std::cout << "Access time: " << accessTime.count() << " ns\n";
  std::cout << "Control time: " << controlTime.count() << " ns\n";
  std::cout << "Ratio: " << float(accessTime.count()) / float(controlTime.count())
            << std::endl;
}

TEST_CASE("Data - WritePerformance", "[data][perf]")  // NOLINT
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
    REQUIRE(tree.values() == points);
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
    REQUIRE(tree.values() == points);
  }

  std::cout << "Write time: " << writeTime.count() << " ns\n";
  std::cout << "Control time: " << controlTime.count() << " ns\n";
  std::cout << "Ratio: " << float(writeTime.count()) / float(controlTime.count())
            << std::endl;
}

TEST_CASE("Data - TreeConversion", "[data][conversion]")  // NOLINT
{
  gal::test::initPythonEnv();
  auto tree = testTree();

  std::cout << "Tree is: \n" << tree << std::endl << "===========\n";

  // Convert tree to a jagged python list.
  py::list lst;
  gal::func::Converter<decltype(tree), py::object>::assign(tree, lst);
  // Convert python jagged list to a cpp jagged vector.
  std::vector<std::vector<std::vector<std::vector<std::vector<int>>>>> jagged;
  gal::func::Converter<py::list, decltype(jagged)>::assign(lst, jagged);

  for (auto v4 : jagged) {
    std::cout << "[";
    for (auto v3 : v4) {
      std::cout << "[";
      for (auto v2 : v3) {
        std::cout << "[";
        for (auto v1 : v2) {
          std::cout << "[";
          for (auto v0 : v1) {
            std::cout << v0 << ", ";
          }
          std::cout << "], ";
        }
        std::cout << "], ";
      }
      std::cout << "], ";
    }
    std::cout << "], ";
  }

  int i = 0;
  REQUIRE(jagged.size() == 2);
  for (auto v4 : jagged) {
    REQUIRE(v4.size() == 2);
    for (auto v3 : v4) {
      REQUIRE(v3.size() == 2);
      for (auto v2 : v3) {
        REQUIRE(v4.size() == 2);
        for (auto v1 : v2) {
          REQUIRE(v1.size() == 2);
          for (int v0 : v1) {
            REQUIRE(v0 == i++);
          }
        }
      }
    }
  }
  REQUIRE(i == 32);

  decltype(tree) tree2;
  gal::func::Converter<py::object, decltype(tree)>::assign(lst, tree2);
  i = 0;
  gal::func::data::ReadView<int, 5> v5(tree2);
  REQUIRE(v5.size() == 2);
  for (auto v4 : v5) {
    REQUIRE(v4.size() == 2);
    for (auto v3 : v4) {
      REQUIRE(v3.size() == 2);
      for (auto v2 : v3) {
        REQUIRE(v4.size() == 2);
        for (auto v1 : v2) {
          REQUIRE(v1.size() == 2);
          for (int v0 : v1) {
            REQUIRE(v0 == i++);
          }
        }
      }
    }
  }
  REQUIRE(i == 32);
}
