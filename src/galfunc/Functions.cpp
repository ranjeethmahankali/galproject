#include <galfunc/Functions.h>
#include <galfunc/MeshFunctions.h>

namespace gal {
namespace func {

// Temp test code.

std::shared_ptr<gal::Mesh> py_loadObjFile(const std::string& filepath)
{
  return std::make_shared<gal::Mesh>(std::move(io::ObjMeshData(filepath, true).toMesh()));
};

BOOST_PYTHON_MODULE(pygalfunc)
{
  using namespace boost::python;
  class_<std::shared_ptr<gal::Mesh>>("Mesh");
  def("meshCentroid", py_meshCentroid);
  def("loadObjFile", py_loadObjFile);
};

}  // namespace func
}  // namespace gal