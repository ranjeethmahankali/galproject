#include <galcore/ObjLoader.h>
#include <galfunc/Functions.h>
#include <galfunc/MapMacro.h>

namespace gal {
namespace func {

glm::vec3 meshCentroid(std::shared_ptr<gal::Mesh> mesh)
{
  return mesh->centroid(gal::eMeshCentroidType::volumeBased);
};

std::shared_ptr<gal::Mesh> loadObjFile(const std::string& filepath)
{
  return std::make_shared<gal::Mesh>(std::move(io::ObjMeshData(filepath, true).toMesh()));
};

BOOST_PYTHON_MODULE(pygalfunc)
{
  using namespace boost::python;
  class_<std::shared_ptr<gal::Mesh>>("Mesh");
  class_<glm::vec3>("vec3")
    .def_readwrite("x", &glm::vec3::x)
    .def_readwrite("y", &glm::vec3::y)
    .def_readwrite("z", &glm::vec3::z);

  def("meshCentroid", meshCentroid);
  def("loadObjFile", loadObjFile);
};

}  // namespace func
}  // namespace gal
