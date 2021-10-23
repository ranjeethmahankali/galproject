#include <gtest/gtest.h>

#include <galcore/ObjLoader.h>
#include <galtest/TestUtils.h>

#ifdef NDEBUG
TEST(ObjLoader, DragonMesh)
#else
TEST(ObjLoader, DISABLED_DragonMesh)
#endif
{
  auto path = GAL_ASSET_DIR / "dragon.obj";
  auto mesh = gal::io::ObjMeshData(path).toMesh();
  ASSERT_EQ(435545, mesh.numVertices());
  ASSERT_EQ(871306, mesh.numFaces());
};

TEST(ObjLoader, BunnyLarge)
{
  auto path = GAL_ASSET_DIR / "bunny_large.obj";
  auto mesh = gal::io::ObjMeshData(path).toMesh();
  ASSERT_EQ(34817, mesh.numVertices());
  ASSERT_EQ(69630, mesh.numFaces());
};

TEST(ObjLoader, BunnySmall)
{
  auto path = GAL_ASSET_DIR / "bunny.obj";
  auto mesh = gal::io::ObjMeshData(path).toMesh();
  ASSERT_EQ(2503, mesh.numVertices());
  ASSERT_EQ(4968, mesh.numFaces());
};
