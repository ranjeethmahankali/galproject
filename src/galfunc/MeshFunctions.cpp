#include <galfunc/MeshFunctions.h>

namespace gal {
namespace func {

boost::python::tuple py_meshCentroid(store::Register meshReg)
{
  return pythonRegisterTuple(meshCentroid(meshReg));
};

types::OutputTuple<3> meshCentroid(const store::Register& meshReg)
{
  using FunctorType = TFunction<TypeList<gal::Mesh>, TypeList<float, float, float>>;
  auto fn           = store::makeFunction<FunctorType>(meshCentroid_impl,
                                             std::array<uint64_t, 1> {meshReg.getId()});

  return types::makeOutputTuple<3>(*fn);
};

TypeList<float, float, float>::SharedTupleType meshCentroid_impl(
  std::shared_ptr<gal::Mesh> mesh)
{
  auto pt = mesh->centroid(gal::eMeshCentroidType::volumeBased);
  return std::make_tuple(std::make_shared<float>(pt.x),
                         std::make_shared<float>(pt.y),
                         std::make_shared<float>(pt.z));
};

}  // namespace func
}  // namespace gal