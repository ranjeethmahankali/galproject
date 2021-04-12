#include <gtest/gtest.h>
#include <galcore/ObjLoader.h>

TEST(ObjLoader, DragonMesh) {
    auto path = gal::utils::absPath("../assets/dragon.obj");
    auto mesh = gal::io::ObjMeshData(path).toMesh();
    ASSERT_EQ(435545, mesh.numVertices());
    ASSERT_EQ(871306, mesh.numFaces());
};