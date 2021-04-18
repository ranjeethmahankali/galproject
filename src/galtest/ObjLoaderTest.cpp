#include <galcore/ObjLoader.h>
#include <gtest/gtest.h>

#ifdef NDEBUG 
TEST(ObjLoader, DragonMesh)
#else
TEST(ObjLoader, DISABLED_DragonMesh)
#endif
{
  auto path = gal::utils::absPath("../assets/dragon.obj");
  auto mesh = gal::io::ObjMeshData(path).toMesh();
  ASSERT_EQ(435545, mesh.numVertices());
  ASSERT_EQ(871306, mesh.numFaces());
};

TEST(ObjLoader, BunnyLarge)
{
  auto path = gal::utils::absPath("../assets/bunny_large.obj");
  auto mesh = gal::io::ObjMeshData(path).toMesh();
  ASSERT_EQ(34817, mesh.numVertices());
  ASSERT_EQ(69630, mesh.numFaces());
};

TEST(ObjLoader, BunnySmall)
{
  auto path = gal::utils::absPath("../assets/bunny.obj");
  auto mesh = gal::io::ObjMeshData(path).toMesh();
  ASSERT_EQ(2503, mesh.numVertices());
  ASSERT_EQ(4968, mesh.numFaces());
};