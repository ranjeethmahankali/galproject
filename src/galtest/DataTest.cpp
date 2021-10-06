#include <gtest/gtest.h>

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
  auto v5   = DataView<int, 5>(tree);
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
