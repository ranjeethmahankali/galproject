#include <galfunc/MeshFunctions.h>

namespace gal {
namespace func {

types::OutputTuple<3> meshCentroid(const store::Register& meshReg)
{
  using FunctorType = TFunction<TypeList<gal::Mesh>, TypeList<float, float, float>>;
  auto fn           = store::makeFunction<FunctorType>(meshCentroid_impl,
                                             std::array<uint64_t, 1> {meshReg.id});

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

types::OutputTuple<1> loadObjFile(const store::Register& filePathReg)
{
  using FunctorType = TFunction<TypeList<std::string>, TypeList<gal::Mesh>>;
  auto fn           = store::makeFunction<FunctorType>(loadObjFile_impl,
                                             std::array<uint64_t, 1> {filePathReg.id});
  return types::makeOutputTuple<1>(*fn);
};

TypeList<gal::Mesh>::SharedTupleType loadObjFile_impl(
  std::shared_ptr<std::string> filepath)
{
  return std::make_tuple(
    std::make_shared<gal::Mesh>(io::ObjMeshData(*filepath, true).toMesh()));
};

}  // namespace func
}  // namespace gal