#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <galcore/ObjLoader.h>
#include <galfunc/Functions.h>
#include <galfunc/MapMacro.h>
#include <boost/python.hpp>

namespace gal {
namespace func {

namespace types {

// clang-format off
template<> struct TypeInfo<bool         > { static constexpr uint32_t id = 0x9566a7b1; };
template<> struct TypeInfo<int32_t      > { static constexpr uint32_t id = 0x9234a3b1; };
template<> struct TypeInfo<float        > { static constexpr uint32_t id = 0x32542672; };
template<> struct TypeInfo<gal::Mesh    > { static constexpr uint32_t id = 0x45342367; };
// clang-format on

}  // namespace types

namespace store {

}

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
