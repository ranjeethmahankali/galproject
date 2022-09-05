#include <gtest/gtest.h>

#include <ObjLoader.h>
#include <TestUtils.h>

#ifdef NDEBUG
TEST(ObjLoader, DragonMesh)
#else
TEST(ObjLoader, DISABLED_DragonMesh)
#endif
{
  auto path = GAL_ASSET_DIR / "dragon.obj";
  std::cout << "Loading mesh from " << path << std::endl;
  auto mesh = gal::io::ObjMeshData(path).toTriMesh();
  ASSERT_EQ(435545, mesh.n_vertices());
  ASSERT_EQ(871306, mesh.n_faces());
};

TEST(ObjLoader, BunnyLarge)
{
  auto path = GAL_ASSET_DIR / "bunny_large.obj";
  std::cout << "Loading mesh from " << path << std::endl;
  auto mesh = gal::io::ObjMeshData(path).toTriMesh();
  ASSERT_EQ(34817, mesh.n_vertices());
  ASSERT_EQ(69630, mesh.n_faces());
};

TEST(ObjLoader, BunnySmall)
{
  auto path = GAL_ASSET_DIR / "bunny.obj";
  std::cout << "Loading mesh from " << path << std::endl;
  auto mesh = gal::io::ObjMeshData(path).toTriMesh();
  ASSERT_EQ(2503, mesh.n_vertices());
  ASSERT_EQ(4968, mesh.n_faces());
};
