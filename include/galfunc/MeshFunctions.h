#pragma once

#include <galcore/ObjLoader.h>
#include <galfunc/Functions.h>

namespace gal {
namespace func {

namespace types {
// clang-format off
template<> struct TypeInfo<gal::Mesh    > { static constexpr uint32_t id = 0x45342367; };
// clang-format on
}  // namespace types

TypeList<float, float, float>::SharedTupleType meshCentroid_impl(
  std::shared_ptr<gal::Mesh> mesh);
types::OutputTuple<3> meshCentroid(const store::Register& meshReg);
boost::python::tuple  py_meshCentroid(store::Register meshReg);

TypeList<gal::Mesh>::SharedTupleType loadObjFile_impl(
  std::shared_ptr<std::string> filepath);
  
types::OutputTuple<1> loadObjFile(const store::Register& filePathReg);
boost::python::tuple py_loadObjFile(store::Register meshReg);

}  // namespace func
}  // namespace gal