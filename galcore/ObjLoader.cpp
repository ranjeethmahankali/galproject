#include <algorithm>
#include <fstream>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include <numeric>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <ObjLoader.h>
#include <Util.h>

namespace gal {
namespace io {

using CStrIter      = std::string::const_iterator;
static auto sLogger = spdlog::stdout_color_mt("objloader");

static std::filesystem::path makeAbsolute(const std::filesystem::path& path)
{
  return path.is_absolute() ? path : std::filesystem::path(utils::absPath(path));
}

ObjMeshData::ObjMeshData(const std::filesystem::path& pathIn, bool flipYZ)
    : mPath(makeAbsolute(pathIn))
    , mFlipYZ(flipYZ)
{
  tinyobj::ObjReaderConfig config;
  config.triangulate = true;
  if (!mReader.ParseFromFile(mPath, config)) {
    if (!mReader.Error().empty()) {
      std::cerr << "TinyObjReader error: " << mReader.Error() << std::endl;
    }
    throw std::filesystem::filesystem_error("Cannot parse file", std::error_code());
  }

  if (!mReader.Warning().empty()) {
    sLogger->warn("TinyObjReader warns: {}", mReader.Warning());
  }
};

TriMesh ObjMeshData::toTriMesh() const
{
  TriMesh     mesh;
  glm::mat4   xform  = glm::rotate(float(M_PI_2), glm::vec3(1.0f, 0.0f, 0.0f));
  const auto& shapes = mReader.GetShapes();
  const auto& attrib = mReader.GetAttrib();

  size_t nfaces = std::accumulate(
    shapes.begin(), shapes.end(), size_t(0), [&](size_t total, const auto& shape) {
      return std::accumulate(
        shape.mesh.num_face_vertices.begin(),
        shape.mesh.num_face_vertices.end(),
        total,
        [&](size_t t2, auto nv) { return t2 + std::max(0, nv - 2); });
    });
  size_t nedges = nfaces * 3 / 2;
  mesh.reserve(attrib.vertices.size() / 3, nedges, nfaces);
  for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
    glm::vec3 v(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]);
    if (mFlipYZ) {
      v = xform * glm::vec4(v, 1.f);
    }
    mesh.add_vertex(v);
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
        size_t                a    = fvi++;
        size_t                b    = fvi++;
        std::array<size_t, 3> fvis = {
          indexOffset + 0,
          indexOffset + a,
          indexOffset + b,
        };
        std::array<TriMesh::VertH, 3> fvs;
        std::transform(fvis.begin(), fvis.end(), fvs.begin(), [&](size_t i) {
          return mesh.vertex_handle(shape.mesh.indices[i].vertex_index);
        });
        mesh.add_face(fvs.begin(), fvs.size());
      }
      indexOffset += nfv;
    }
  }
  return mesh;
}

}  // namespace io
}  // namespace gal
