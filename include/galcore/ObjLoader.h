#pragma once
#include <galcore/Mesh.h>
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
    uint32_t vertices[3];
    uint32_t texCoords[3];
    uint32_t normals[3];
  };

  ObjMeshData(const std::filesystem::path& filepath);

  Mesh toMesh() const;

private:
  std::filesystem::path  mPath;
  std::vector<glm::vec3> mVertices;
  std::vector<glm::vec3> mNormals;
  std::vector<glm::vec2> mTexCoords;
  std::vector<Face>      mFaces;
};

}  // namespace io
}  // namespace gal