#pragma once

#include <OpenMesh/Core/Mesh/Attributes.hh>
#include <OpenMesh/Core/Utils/Property.hh>
#include <filesystem>
#include <glm/detail/qualifier.hpp>
#include <glm/geometric.hpp>
#include <limits>
#include <span>
#include <unordered_map>

#include <galcore/Box.h>
#include <galcore/OpenMeshAdaptor.h>
#include <galcore/Plane.h>
#include <galcore/RTree.h>
#include <galcore/Sphere.h>
#include <galcore/Util.h>

using EdgeType     = gal::IndexPair;
using EdgeTypeHash = gal::IndexPairHash;

namespace gal {

struct MeshTraits : public OpenMesh::DefaultTraits
{
  // Use glm for everything.
  typedef glm::vec3 Point;
  typedef glm::vec3 Normal;
  typedef float     TexCoord1D;
  typedef glm::vec2 TexCoord2D;
  typedef glm::vec3 TexCoord3D;
  typedef int       TextureIndex;
  typedef glm::vec3 Color;

  VertexAttributes(OpenMesh::Attributes::Normal);
  HalfedgeAttributes(OpenMesh::Attributes::PrevHalfedge | OpenMesh::Attributes::Status);
  EdgeAttributes(OpenMesh::Attributes::Status | OpenMesh::Attributes::Normal);
  FaceAttributes(OpenMesh::Attributes::Normal | OpenMesh::Attributes::Status);
};

enum class eMeshCentroidType
{
  vertexBased = 0,
  areaBased,
  volumeBased
};

enum class eMeshElement
{
  vertex,
  face
};

struct TriMesh : public OpenMesh::TriMesh_ArrayKernelT<MeshTraits>
{
  using BaseMesh = OpenMesh::TriMesh_ArrayKernelT<MeshTraits>;
  using FaceH    = OpenMesh::FaceHandle;
  using VertH    = OpenMesh::VertexHandle;
  using HalfH    = OpenMesh::HalfedgeHandle;
  using EdgeH    = OpenMesh::EdgeHandle;

  TriMesh();

  bool      isSolid() const;
  float     area() const;
  gal::Box3 bounds() const;
  float     volume() const;
  bool      contains(const glm::vec3& pt) const;
  glm::vec3 closestPoint(const glm::vec3& pt, float maxDistance = FLT_MAX) const;
  TriMesh   clippedWithPlane(const Plane& plane) const;
  void      transform(const glm::mat4& mat);
  void      set_color(VertH v, const glm::vec3& c);
  glm::vec3 color(VertH v) const;
  TriMesh   subMesh(std::span<const int> faces) const;
  void      updateRTrees() const;

private:
  mutable utils::Cached<RTree3d> mFaceTree;
  mutable utils::Cached<RTree3d> mVertexTree;

  const RTree3d&           elementTree(eMeshElement etype) const;
  std::array<glm::vec3, 3> facePoints(FaceH f) const;
  glm::vec3                vertexCentroid() const;
  glm::vec3                areaCentroid() const;
  glm::vec3                volumeCentroid() const;

public:
  template<typename IntInserter>
  void queryBox(const gal::Box3& box, IntInserter inserter, eMeshElement etype) const
  {
    updateRTrees();
    elementTree(etype).queryBoxIntersects(box, inserter);
  }

  template<typename IntInserter>
  void querySphere(const gal::Sphere& sphere,
                   IntInserter        inserter,
                   eMeshElement       etype) const
  {
    updateRTrees();
    elementTree(etype).queryByDistance(sphere.center, sphere.radius, inserter);
  }

  glm::vec3 centroid(eMeshCentroidType ctype = eMeshCentroidType::vertexBased) const;
};

TriMesh makeRectangularMesh(const gal::Plane& plane,
                            const gal::Box2&  box,
                            float             edgelength);

template<>
struct Serial<TriMesh> : public std::true_type
{
  static TriMesh deserialize(Bytes& bytes) { throw std::logic_error("Not Implemented"); }
  static Bytes   serialize(const TriMesh& msh)
  {
    throw std::logic_error("Not Implemented");
  }
};

}  // namespace gal