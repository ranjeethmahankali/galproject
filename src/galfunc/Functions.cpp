#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <galcore/ObjLoader.h>
#include <galfunc/Functions.h>
#include <galfunc/MapMacro.h>

namespace gal {
namespace func {

// Temp test code.

types::OutputTuple<3> meshCentroid(const store::Register& meshReg)
{
  using FunctorType = TFunction<TypeList<gal::Mesh>, TypeList<float, float, float>>;
  auto fn           = store::makeFunction<FunctorType>(
    [](std::shared_ptr<gal::Mesh> mesh) -> FunctorType::OutputsType {
      auto pt = mesh->centroid(gal::eMeshCentroidType::volumeBased);
      return std::make_tuple(std::make_shared<float>(pt.x),
                             std::make_shared<float>(pt.y),
                             std::make_shared<float>(pt.z));
    },
    std::array<uint64_t, 1> {meshReg.getId()});

  return types::makeOutputTuple<3>(*fn);
};

boost::python::tuple py_meshCentroid(std::shared_ptr<gal::Mesh> mesh)
{
  // sMeshCentroid.run()
  auto tup = boost::python::make_tuple();
  return boost::python::make_tuple();
};

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