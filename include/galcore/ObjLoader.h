#pragma once
#include <galcore/Mesh.h>
#include <tiny_obj_loader.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <vector>

namespace gal {
namespace io {

class ObjMeshData
{
public:
  struct Face
  {
    int64_t vertices[3];
    int64_t texCoords[3];
    int64_t normals[3];
  };

  ObjMeshData(const std::filesystem::path& filepath, bool flipYAndZ = false);

  Mesh toMesh() const;

private:
  std::filesystem::path mPath;
  tinyobj::ObjReader mReader;
  bool mFlipYZ = false;
};

}  // namespace io
}  // namespace gal