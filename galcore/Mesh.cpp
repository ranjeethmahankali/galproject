#define _USE_MATH_DEFINES

#include <OpenMesh/Core/IO/MeshIO.hh>

#include <Mesh.h>

#include <math.h>
#include <array>
#include <atomic>
#include <mutex>
#include <numeric>
#include <stdexcept>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for_each.h>
#include <tbb/parallel_reduce.h>
#include <OpenMesh/Core/Utils/Property.hh>
#include <boost/range/adaptors.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>

#include <Box.h>
#include <RTree.h>
#include <Util.h>

namespace gal {

static float triangleArea(const std::array<glm::vec3, 3>& fvs)
{
  return glm::length(glm::cross(fvs[1] - fvs[0], fvs[2] - fvs[0])) * 0.5f;
}

static float tetVolume(const std::array<glm::vec3, 3>& fvs)
{
  return std::abs(glm::dot(glm::cross(fvs[0], fvs[1]), fvs[2]) / 6.0f);
}

template<typename MeshT>
void initVertexColors(MeshT& mesh)
{
  for (auto vh : mesh.vertices()) {
    mesh.set_color(vh, MeshTraits::Color {1.f, 1.f, 1.f});
  }
}

TriMesh::TriMesh()
    : mFaceTree()
    , mVertexTree()
{
  initVertexColors(*this);
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
  static constexpr float s1_6th = 1.f / 6.f;
  glm::vec3              refpt  = bounds().center();
  std::vector<glm::vec3> jvs(n_vertices());
  tbb::parallel_for_each(vertices(), [&](VertH v) { jvs[v.idx()] = point(v) - refpt; });
  return std::accumulate(faces_begin(), faces_end(), 0.f, [&](float total, FaceH f) {
    glm::vec3                jv = jvs[cfv_begin(f)->idx()];
    std::array<glm::vec3, 3> vs;
    std::transform(
      cfv_begin(f), cfv_end(f), vs.begin(), [&](VertH v) { return point(v); });
    return total + glm::dot(jv, glm::cross(vs[1] - vs[0], vs[2] - vs[0])) * s1_6th;
  });
}

bool TriMesh::contains(const glm::vec3& pt) const
{
  throw std::logic_error("Not Implemented");
}

static void faceClosestPt(const std::array<glm::vec3, 3>& vs,
                          const glm::vec3&                pt,
                          glm::vec3&                      closept,
                          float&                          bestdsq)
{
  const glm::vec3& v0       = vs[0];
  glm::vec3        fnorm    = glm::normalize(glm::cross(vs[1] - vs[0], vs[2] - vs[0]));
  glm::vec3        proj     = fnorm * glm::dot((v0 - pt), fnorm);
  float            planedsq = glm::length2(proj);
  if (planedsq > bestdsq) {
    return;
  }
  glm::vec3 projpt   = pt + proj;
  uint32_t  nOutside = 0;
  for (uint32_t i = 0; i < 3; i++) {
    const glm::vec3& v1 = vs[i];
    const glm::vec3& v2 = vs[(i + 1) % 3];
    bool outside        = glm::dot(glm::cross(v1 - projpt, v2 - projpt), fnorm) < 0.f;
    if (outside) {
      nOutside++;
      glm::vec3 ln  = v2 - v1;
      float     r   = std::clamp(glm::dot(ln, projpt - v1) / glm::length2(ln), 0.f, 1.f);
      glm::vec3 cpt = v2 * r + v1 * (1.f - r);
      float     dsq = glm::length2(cpt - pt);
      if (dsq < bestdsq) {
        closept = cpt;
        bestdsq = dsq;
      }
    }
    if (nOutside > 1) {
      break;
    }
  }
  if (nOutside == 0) {
    closept = projpt;
    bestdsq = planedsq;
  }
}

glm::vec3 TriMesh::closestPoint(const glm::vec3& pt, float maxd) const
{
  int nearvi = -1;
  mVertexTree->queryNearestN(pt, 1, &nearvi);
  if (nearvi == -1) {
    return vec3_unset;
  }
  glm::vec3 closept = point(vertex_handle(nearvi));
  float     bestdsq = glm::length2(pt - closept);
  float     vdist   = std::sqrt(bestdsq);
  if (vdist > maxd) {
    return vec3_unset;
  }
  glm::vec3 halfdiag(vdist, vdist, vdist);
  // TODO: Avoid after refactoring the rtree to return a lazy iterator.
  std::vector<int> candidates;
  mFaceTree->queryBoxIntersects(Box3(pt - halfdiag, pt + halfdiag),
                                std::back_inserter(candidates));
  for (int fi : candidates) {
    auto fvs = facePoints(face_handle(fi));
    faceClosestPt(fvs, pt, closept, bestdsq);
  }
  return closept;
}

TriMesh TriMesh::clippedWithPlane(const Plane& plane) const
{
  static constexpr uint8_t                               X = UINT8_MAX;
  static constexpr std::array<std::array<uint8_t, 6>, 8> sTriIndices {{
    {X, X, X, X, X, X},
    {0, 3, 5, X, X, X},
    {3, 1, 4, X, X, X},
    {0, 1, 5, 1, 4, 5},
    {4, 2, 5, X, X, X},
    {0, 3, 4, 0, 4, 2},
    {1, 5, 3, 1, 2, 5},
    {0, 1, 2, X, X, X},
  }};
  static constexpr std::array<uint8_t, 8> sNumVerts {0, 3, 3, 6, 3, 6, 6, 3};
  static constexpr std::array<uint8_t, 8> sNumHedges {0, 1, 1, 1, 2, 2, 2, 0};
  struct VData
  {
    float distance = 0.f;  // Away from the plane.
    VertH mappedV  = VertH();
    bool  inside   = false;
  };
  glm::vec3            origin = plane.origin();
  glm::vec3            unorm  = glm::normalize(plane.normal());
  std::atomic_uint32_t nverts = 0;
  std::vector<VData>   vdata(n_vertices());
  tbb::parallel_for_each(vertices(), [&](VertH v) {
    auto& vd    = vdata[v.idx()];
    vd.distance = glm::dot(point(v) - origin, unorm);
    vd.inside   = vd.distance < 0.f;
    if (vd.inside) {
      nverts++;
    }
  });
  TriMesh clipped;
  if (nverts > 0 && n_vertices() > 0) {
    float r = float(nverts) / float(n_vertices());
    // Estimate face and edge counts based on the vertex count.
    clipped.reserve(nverts,
                    size_t(std::round(r * float(n_edges()))),
                    size_t(std::round(r * float(size_t(n_faces())))));
  }
  std::vector<VertH> edgepts(n_edges());
  for (auto f : faces()) {
    uint8_t              fvi = 0;
    uint8_t              fe  = 0;
    std::array<EdgeH, 3> fedges;
    std::array<VertH, 3> fverts;
    for (auto it = cfh_begin(f); it != cfh_end(f) && fvi < 3; it++, fvi++) {
      HalfH h     = *it;
      VertH fv    = from_vertex_handle(h);
      fedges[fvi] = edge_handle(h);
      fverts[fvi] = fv;
      fe |= (1 << fvi) * uint8_t(vdata[fv.idx()].inside);
    }
    std::array<VertH, 6> tempV;
    const uint8_t* const row = sTriIndices[fe].data();
    std::transform(row, row + sNumVerts[fe], tempV.begin(), [&](const uint8_t vi) {
      VertH v;
      if (vi > 2) {
        EdgeH  e  = fedges[vi - 3];
        VertH& ev = edgepts[e.idx()];
        if (!ev.is_valid()) {
          VertH a  = to_vertex_handle(halfedge_handle(e, 0));
          VertH b  = to_vertex_handle(halfedge_handle(e, 1));
          auto& ad = vdata[a.idx()];
          auto& bd = vdata[b.idx()];
          assert(ad.inside != bd.inside);
          float r = bd.distance / (bd.distance - ad.distance);
          ev      = clipped.add_vertex(point(a) * r + point(b) * (1.f - r));
        }
        return ev;
      }
      else {
        VertH  oldv = fverts[vi];
        VertH& newv = vdata[oldv.idx()].mappedV;
        if (!newv.is_valid()) {
          newv = clipped.add_vertex(point(oldv));
        }
        return newv;
      }
    });
    for (size_t vi = 0; vi < sNumVerts[fe]; vi += 3) {
      clipped.add_face(tempV[vi], tempV[vi + 1], tempV[vi + 2]);
    }
  }
  return clipped;
}

void TriMesh::transform(const glm::mat4& mat)
{
  tbb::parallel_for_each(
    vertices(), [&](VertH v) { point(v) = glm::vec3(mat * glm::vec4(point(v), 1.f)); });
  // TODO: The two lines below can be run in parallel.
  mFaceTree.expire();
  mVertexTree.expire();
  update_normals();
}

TriMesh TriMesh::subMesh(std::span<const int> faces) const
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

void TriMesh::updateRTrees() const
{
  {
    std::lock_guard lock(mFaceTree.mutex());
    if (!mFaceTree) {
      mFaceTree->clear();
      for (FaceH f : faces()) {
        auto fvs = facePoints(f);
        mFaceTree->insert(Box3(fvs), size_t(f.idx()));
      }
      mFaceTree.unexpire();
    }
  }
  {
    std::lock_guard lock(mVertexTree.mutex());
    if (!mVertexTree) {
      mVertexTree->clear();
      for (VertH v : vertices()) {
        mVertexTree->insert(Box3(point(v)), size_t(v.idx()));
      }
      mVertexTree.unexpire();
    }
  }
}

void TriMesh::setVertexColor(glm::vec3 color)
{
  for (TriMesh::VertH v : vertices()) {
    set_color(v, color);
  }
}

template<typename MeshT>
void flipYZAxes(MeshT& mesh)
{
  static glm::mat4 xform = glm::rotate(float(M_PI_2), glm::vec3(1.0f, 0.0f, 0.0f));
  for (auto vh : mesh.vertices()) {
    mesh.point(vh) = glm::vec3(xform * glm::vec4(mesh.point(vh), 1.f));
  }
}

template<typename MeshT>
MeshT loadMeshFromFile(const fs::path& path, bool flipYZ)
{
  MeshT mesh;
  OpenMesh::IO::read_mesh(mesh, path.string());
  if (flipYZ) {
    flipYZAxes(mesh);
  }
  initVertexColors(mesh);
  return mesh;
}

TriMesh TriMesh::loadFromFile(const fs::path& path, bool flipYZ)
{
  return loadMeshFromFile<TriMesh>(path, flipYZ);
}

const RTree3d& TriMesh::elementTree(eMeshElement type) const
{
  switch (type) {
  case eMeshElement::face:
    return *mFaceTree;
  case eMeshElement::vertex:
    return *mVertexTree;
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
  TriMesh    mesh;
  size_t     nverts = vdims.x * vdims.y;
  size_t     nfaces = qdims.x * qdims.y * 2;  // 2x triangles than quads.
  size_t     nedges = qdims.y * vdims.x +     // Edges along y
                  qdims.x * vdims.y +         // Edges along x
                  qdims.x * qdims.y;          // Diagonals, 1 per quad.
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

PolyMesh::PolyMesh()
{
  initVertexColors(*this);
}

gal::Box3 PolyMesh::bounds() const
{
  namespace ba = boost::adaptors;
  auto vs      = vertices() | ba::transformed([&](VertH v) { return point(v); });
  return Box3::create(vs.begin(), vs.end());
}

PolyMesh PolyMesh::loadFromFile(const fs::path& path, bool flipYZ)
{
  return loadMeshFromFile<PolyMesh>(path, flipYZ);
}

void PolyMesh::transform(const glm::mat4& mat)
{
  tbb::parallel_for_each(
    vertices(), [&](VertH v) { point(v) = glm::vec3(mat * glm::vec4(point(v), 1.f)); });
}

}  // namespace gal
