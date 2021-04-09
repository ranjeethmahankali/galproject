#pragma once

#include <galcore/ObjLoader.h>
#include <galfunc/Functions.h>

TYPE_INFO(gal::Mesh, 0x45342367);

namespace gal {
namespace func {

TypeList<float, float, float>::SharedTupleType meshCentroid_impl(
  std::shared_ptr<gal::Mesh> mesh);
types::OutputTuple<3> meshCentroid(const store::Register& meshReg);

TypeList<gal::Mesh>::SharedTupleType loadObjFile_impl(
  std::shared_ptr<std::string> filepath);
  
types::OutputTuple<1> loadObjFile(const store::Register& filePathReg);

}  // namespace func
}  // namespace gal