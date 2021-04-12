#include <galfunc/Functions.h>
#include <galfunc/MeshFunctions.h>
#include <galfunc/Variable.h>
#include <gtest/gtest.h>

TEST(MeshFunction, Centroid)
{
  using namespace gal::func;
  using namespace std::string_literals;
  auto [path] = variable<std::string>("/home/rnjth94/dev/GeomAlgoLib/assets/bunny.obj"s);
  auto [mesh] = loadObjFile(path);
  auto [pt]   = meshCentroid(mesh);

  auto ptval = *(store::get<glm::vec3>(pt.id));

  std::cout << "(" << ptval.x << ", " << ptval.y << ", " << ptval.z << ")\n";
};