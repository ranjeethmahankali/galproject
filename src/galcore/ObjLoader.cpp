#include <galcore/DebugProfile.h>
#include <galcore/ObjLoader.h>
#include <galcore/Util.h>
#include <algorithm>
#include <fstream>
#include <glm/gtx/transform.hpp>
#include <iostream>

namespace gal {
namespace io {

using CStrIter = std::string::const_iterator;

static std::filesystem::path makeAbsolute(const std::filesystem::path& path)
{
  return path.is_absolute() ? path : std::filesystem::path(utils::absPath(path));
}

ObjMeshData::ObjMeshData(const std::filesystem::path& pathIn, bool flipYZ)
    : mPath(makeAbsolute(pathIn))
    , mFlipYZ(flipYZ)
{
  GALSCOPE(__func__);
  tinyobj::ObjReaderConfig config;
  config.triangulate = true;
  if (!mReader.ParseFromFile(mPath, config)) {
    if (!mReader.Error().empty()) {
      std::cerr << "TinyObjReader error: " << mReader.Error() << std::endl;
    }
    throw std::filesystem::filesystem_error("Cannot parse file", std::error_code());
  }

  if (!mReader.Warning().empty()) {
    std::cout << "TinyObjReader warning: " << mReader.Warning();
  }
};

Mesh ObjMeshData::toMesh() const
{
  GALSCOPE(__func__);
  std::vector<size_t> indices;

  const auto&        shapes     = mReader.GetShapes();
  const auto&        attrib     = mReader.GetAttrib();
  std::vector<float> vertCoords = attrib.vertices;

  glm::mat4 xform = glm::rotate(float(M_PI_2), glm::vec3(1.0f, 0.0f, 0.0f));
  if (vertCoords.size() % 3) {
    throw std::out_of_range("Invalid coordinate array");
  }
  static_assert(sizeof(glm::vec3) == 3 * sizeof(float), "Alignment problem");
  if (mFlipYZ) {
    glm::vec3* vptr   = (glm::vec3*)vertCoords.data();
    size_t     nVerts = vertCoords.size() / 3;
    for (size_t vi = 0; vi < nVerts; vi++) {
      *vptr = glm::vec3(xform * glm::vec4 {vptr->x, vptr->y, vptr->z, 1.0f});
      vptr++;
    }
  }

  for (const auto& shape : shapes) {
    size_t indexOffset = 0;
    for (size_t fi = 0; fi < shape.mesh.num_face_vertices.size(); fi++) {
      size_t nfv = size_t(shape.mesh.num_face_vertices[fi]);
      if (nfv < 3) {
        continue;
      }
      size_t fvi = 1;
      while (fvi < nfv) {
        size_t a = fvi++;
        size_t b = fvi++;
        indices.push_back(size_t(shape.mesh.indices[indexOffset + 0].vertex_index));
        indices.push_back(size_t(shape.mesh.indices[indexOffset + a].vertex_index));
        indices.push_back(size_t(shape.mesh.indices[indexOffset + b].vertex_index));
      }
      indexOffset += nfv;
    }
  }

  return Mesh(
    vertCoords.data(), vertCoords.size() / 3, indices.data(), indices.size() / 3);
}

}  // namespace io
}  // namespace gal