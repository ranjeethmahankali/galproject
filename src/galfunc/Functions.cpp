#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <galcore/ObjLoader.h>
#include <galfunc/Functions.h>
#include <galfunc/MapMacro.h>
#include <boost/python.hpp>

namespace gal {
namespace func {

boost::python::tuple meshCentroid(std::shared_ptr<gal::Mesh> mesh)
{
  auto pt = mesh->centroid(gal::eMeshCentroidType::volumeBased);
  return boost::python::make_tuple(pt.x, pt.y, pt.z);
};

std::shared_ptr<gal::Mesh> loadObjFile(const std::string& filepath)
{
  return std::make_shared<gal::Mesh>(std::move(io::ObjMeshData(filepath, true).toMesh()));
};

BOOST_PYTHON_MODULE(pygalfunc)
{
  using namespace boost::python;
  class_<std::shared_ptr<gal::Mesh>>("Mesh");
  def("meshCentroid", meshCentroid);
  def("loadObjFile", loadObjFile);
};

}  // namespace func
}  // namespace gal
