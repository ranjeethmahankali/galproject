#define _USE_MATH_DEFINES

#include <math.h>
#include <array>
#include <numeric>
#include <stdexcept>

#include <tbb/parallel_for_each.h>
#include <boost/range/adaptors.hpp>

#include <galcore/Box.h>
#include <galcore/Mesh.h>
#include <galcore/ObjLoader.h>
#include <galcore/RTree.h>

namespace gal {

static constexpr uint8_t                               X = UINT8_MAX;
static constexpr std::array<std::array<uint8_t, 6>, 8> sClipTriTable {{
  {X, X, X, X, X, X},
  {0, 3, 5, X, X, X},
  {3, 1, 4, X, X, X},
  {0, 1, 5, 1, 4, 5},
  {4, 2, 5, X, X, X},
  {0, 3, 4, 0, 4, 2},
  {1, 5, 3, 1, 2, 5},
  {0, 1, 2, X, X, X},
}};

static constexpr std::array<uint8_t, 8> sClipVertCountTable {0, 3, 3, 6, 3, 6, 6, 3};

static float triangleArea(const std::array<glm::vec3, 3>& fvs)
{
  return glm::length(glm::cross(fvs[1] - fvs[0], fvs[2] - fvs[0])) * 0.5f;
}

static float tetVolume(const std::array<glm::vec3, 3>& fvs)
{
  return std::abs(glm::dot(glm::cross(fvs[0], fvs[1]), fvs[2]) / 6.0f);
}

bool TriMesh::isSolid() const
{
  return std::all_of(halfedges_begin(), halfedges_end(), [&](HalfH h) {
    return face_handle(h).is_valid();
  });
}

float TriMesh::area() const
{
  return std::accumulate(faces_begin(), faces_end(), 0.f, [&](float total, FaceH f) {
    return total + triangleArea(facePoints(f));
  });
}

gal::Box3 TriMesh::bounds() const
{
  namespace ba = boost::adaptors;
  auto vs      = vertices() | ba::transformed([&](VertH v) { return point(v); });
  return Box3::create(vs.begin(), vs.end());
}

float TriMesh::volume() const
{
  if (!isSolid()) {
    return 0.f;
  }

  glm::vec3              refpt = bounds().center();
  std::vector<glm::vec3> jvs(n_vertices());
  std::transform(vertices_begin(), vertices_end(), jvs.begin(), [&](VertH v) {
    return point(v) - refpt;
  });
  return std::accumulate(faces_begin(), faces_end(), 0.f, [&](float total, FaceH f) {
    std::array<glm::vec3, 3> js;
    std::transform(
      cfv_begin(f), cfv_end(f), js.begin(), [&](VertH v) { return jvs[v.idx()]; });
    return total + tetVolume(js) * utils::sign(glm::dot(js[0], normal(f)));
  });
}

bool TriMesh::contains(const glm::vec3& pt) const
{
  throw std::logic_error("Not Implemented");
}

glm::vec3 TriMesh::closestPoint(const glm::vec3& pt, float maxDistance) const
{
  throw std::logic_error("Not Implemented");
}

TriMesh TriMesh::clippedWithPlane(const Plane& plane) const
{
  throw std::logic_error("Not Implemented");
}

void TriMesh::transform(const glm::mat4& mat)
{
  tbb::parallel_for_each(
    vertices(), [&](VertH v) { point(v) = glm::vec3(mat * glm::vec4(point(v), 1.f)); });
  // TODO: The two lines below can be run in parallel.
  initRTrees();
  update_normals();
}

TriMesh TriMesh::subMesh(const std::span<int>& faces) const
{
  std::vector<VertH> newVerts(n_vertices());
  TriMesh            smesh;
  smesh.reserve(faces.size() * 3, faces.size() * 3 / 2, faces.size());
  for (int fi : faces) {
    FaceH                fh = face_handle(fi);
    std::array<VertH, 3> fvs;
    std::transform(cfv_begin(fh), cfv_end(fh), fvs.begin(), [&](VertH vh) {
      VertH& nv = newVerts[vh.idx()];
      if (!nv.is_valid()) {
        nv = smesh.add_vertex(point(vh));
      }
      return nv;
    });
    smesh.add_face(fvs.data(), fvs.size());
  }
  return smesh;
}

void TriMesh::initRTrees()
{
  mFaceTree.clear();
  for (FaceH f : faces()) {
    auto fvs = facePoints(f);
    mFaceTree.insert(Box3(fvs), size_t(f.idx()));
  }
  mVertexTree.clear();
  for (VertH v : vertices()) {
    mVertexTree.insert(Box3(point(v)), size_t(v.idx()));
  }
}

const RTree3d& TriMesh::elementTree(eMeshElement type) const
{
  switch (type) {
  case eMeshElement::face:
    return mFaceTree;
  case eMeshElement::vertex:
    return mVertexTree;
  default:
    throw std::runtime_error("Invalid element type");
  }
}

std::array<glm::vec3, 3> TriMesh::facePoints(FaceH f) const
{
  std::array<glm::vec3, 3> fvs;
  std::transform(
    cfv_begin(f), cfv_end(f), fvs.begin(), [&](VertH v) { return point(v); });
  return fvs;
}

glm::vec3 TriMesh::vertexCentroid() const
{
  namespace ba = boost::adaptors;
  auto vs      = vertices() | ba::transformed([&](VertH v) { return point(v); });
  return std::accumulate(vs.begin(), vs.end(), glm::vec3(0.f)) / float(n_vertices());
}

glm::vec3 TriMesh::areaCentroid() const
{
  glm::vec3 vsum(0.f);
  float     wsum = 0.f;
  for (FaceH f : faces()) {
    auto  fvs = facePoints(f);
    float w   = triangleArea(fvs);
    vsum +=
      w * std::accumulate(fvs.begin(), fvs.end(), glm::vec3(0.f)) / float(fvs.size());
    wsum += w;
  }
  return vsum / wsum;
}

glm::vec3 TriMesh::volumeCentroid() const
{
  glm::vec3              refpt = bounds().center();
  std::vector<glm::vec3> jvs(n_vertices());
  std::transform(vertices_begin(), vertices_end(), jvs.begin(), [&](VertH v) {
    return point(v) - refpt;
  });
  glm::vec3 vsum(0.f);
  float     wsum = 0.f;
  for (FaceH f : faces()) {
    std::array<glm::vec3, 3> fvs = facePoints(f);
    std::array<glm::vec3, 3> js;
    std::transform(
      cfv_begin(f), cfv_end(f), js.begin(), [&](VertH v) { return jvs[v.idx()]; });
    float w = tetVolume(js);
    vsum += w * (refpt + std::accumulate(fvs.begin(), fvs.end(), glm::vec3(0.f))) * 0.25f;
    wsum += w;
  }
  return vsum / wsum;
}

glm::vec3 TriMesh::centroid(eMeshCentroidType ctype) const
{
  switch (ctype) {
  case eMeshCentroidType::vertexBased:
    return vertexCentroid();
  case eMeshCentroidType::areaBased:
    return areaCentroid();
  case eMeshCentroidType::volumeBased:
    return volumeCentroid();
  default:
    assert(false);
    return glm::vec3(0.f);
  }
}

TriMesh makeRectangularMesh(const gal::Plane& plane, const gal::Box2& box, float edgeln)
{
  glm::vec2  diag  = box.diagonal();
  glm::ivec2 qdims = glm::ivec2(glm::ceil(diag / float(edgeln)));
  glm::vec2  qsize(diag.x / float(qdims.x), diag.y / float(qdims.y));
  // One more vertex than quad in each direction.
  glm::ivec2 vdims = qdims + glm::ivec2(1);

  TriMesh mesh;
  size_t  nverts = vdims.x * vdims.y;
  size_t  nfaces = qdims.x * qdims.y * 2;  // 2x triangles than quads.
  size_t  nedges = qdims.y * vdims.x +     // Edges along y
                  qdims.x * vdims.y +      // Edges along x
                  qdims.x * qdims.y;       // Diagonals, 1 per quad.
  mesh.reserve(nverts, nedges, nfaces);
  for (glm::ivec2 vi = glm::ivec2(0); vi.y < vdims.y; vi.y++) {
    for (vi.x = 0; vi.x < vdims.x; vi.x++) {
      mesh.add_vertex(plane.origin() +
                      (plane.xaxis() * (qsize.x * float(vi.x) + box.min.x)) +
                      (plane.yaxis() * (qsize.y * float(vi.y) + box.min.y)));
    }
  }

  for (glm::ivec2 qi = glm::ivec2(0); qi.y < qdims.y; qi.y++) {
    for (qi.x = 0; qi.x < qdims.x; qi.x++) {
      std::array<TriMesh::VertH, 4> quadIndices = {
        mesh.vertex_handle(qi.x + qi.y * (qdims.x + 1)),
        mesh.vertex_handle(qi.x + qi.y * (qdims.x + 1) + 1),
        mesh.vertex_handle(qi.x + (qi.y + 1) * (qdims.x + 1)),
        mesh.vertex_handle(qi.x + (qi.y + 1) * (qdims.x + 1) + 1)};
      mesh.add_face(quadIndices[0], quadIndices[1], quadIndices[2]);
      mesh.add_face(quadIndices[1], quadIndices[3], quadIndices[2]);
    }
  }

  return mesh;
}

}  // namespace gal
