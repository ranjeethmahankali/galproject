#pragma once
#include <galcore/Box.h>
#include <galcore/RTree.h>
#include <galcore/Util.h>
#include <filesystem>
#include <limits>
#include <unordered_map>

#include <galcore/Plane.h>

using EdgeType     = gal::IndexPair;
using EdgeTypeHash = gal::IndexPairHash;

namespace gal {

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

class Mesh
{
public:
  struct Face
  {
    static const Face unset;
    union
    {
      struct
      {
        size_t a, b, c;
      };
      size_t indices[3];
    };

    Face();
    Face(size_t v1, size_t v2, size_t v3);
    Face(size_t const indices[3]);

    void     flip();
    EdgeType edge(uint8_t edgeIndex) const;
    bool     containsVertex(size_t vertIndex) const;
    bool     isDegenerate() const;
  };

  struct EdgeTriplet
  {
    size_t a = SIZE_MAX, b = SIZE_MAX, c = SIZE_MAX;

    EdgeTriplet() = default;
    EdgeTriplet(size_t const (&indices)[3]);
    EdgeTriplet(size_t, size_t, size_t);

    void set(size_t, size_t, size_t);
    void set(size_t);
  };

private:
  using ConstVertIter = std::vector<glm::vec3>::const_iterator;
  using ConstFaceIter = std::vector<Face>::const_iterator;

  std::vector<glm::vec3> mVertices;
  std::vector<Face>      mFaces;

  /*Maps vertex indices to indices of connected faces.*/
  std::vector<std::vector<size_t>> mVertFaces;
  /*Maps the vertex indices to the indices of connected edges.*/
  std::vector<std::vector<size_t>> mVertEdges;
  /*Maps the vertex-index-pair to the index of the edge connecting those
   * vertices.*/
  std::unordered_map<EdgeType, size_t, EdgeTypeHash> mEdgeIndexMap;

  /*Maps the edge index to the pair of connected vertex indices.*/
  std::vector<EdgeType> mEdges;
  /*Maps the edge index to the indices of the connected faces.*/
  std::vector<std::vector<size_t>> mEdgeFaces;
  /*Maps the face index to the indices of the 3 edges connected to that face.*/
  std::vector<EdgeTriplet> mFaceEdges;

  std::vector<glm::vec3> mVertexNormals;
  std::vector<glm::vec3> mFaceNormals;
  bool                   mIsSolid;
  RTree3d                mFaceTree;
  RTree3d                mVertexTree;

  void  computeCache();
  void  computeTopology();
  void  computeRTrees();
  void  computeNormals();
  void  addEdge(const Face&, size_t fi, uint8_t, size_t&);
  void  addEdges(const Face&, size_t fi);
  float faceArea(const Face& f) const;
  void  getFaceCenter(const Face& f, glm::vec3& center) const;
  void  checkSolid();

  glm::vec3      areaCentroid() const;
  glm::vec3      volumeCentroid() const;
  const RTree3d& elementTree(eMeshElement element) const;

  void faceClosestPt(size_t           faceIndex,
                     const glm::vec3& pt,
                     glm::vec3&       closePt,
                     float&           bestSqDist) const;

public:
  Mesh(const Mesh& other);
  Mesh(const glm::vec3* verts, size_t nVerts, const Face* faces, size_t nFaces);
  Mesh(const std::vector<glm::vec3>& verts, const std::vector<Face>& faces);
  Mesh(std::vector<glm::vec3>&& verts, std::vector<Face>&& faces);
  Mesh(const float*  vertCoords,
       size_t        nVerts,
       const size_t* faceVertIndices,
       size_t        nFaces);

  size_t              numVertices() const noexcept;
  size_t              numFaces() const noexcept;
  glm::vec3           vertex(size_t vi) const;
  Face                face(size_t fi) const;
  glm::vec3           vertexNormal(size_t vi) const;
  const glm::vec3&    faceNormal(size_t fi) const;
  Mesh::ConstVertIter vertexCBegin() const;
  Mesh::ConstVertIter vertexCEnd() const;
  Mesh::ConstFaceIter faceCBegin() const;
  Mesh::ConstFaceIter faceCEnd() const;

  gal::box3 bounds() const;
  float     faceArea(size_t fi) const;
  float     area() const;
  gal::box3 faceBounds(size_t fi) const;

  float     volume() const;
  bool      isSolid() const;
  glm::vec3 centroid() const;
  glm::vec3 centroid(const eMeshCentroidType centroid_type) const;

  bool contains(const glm::vec3& pt) const;

  void clipWithPlane(const Plane& plane);

  void transform(const glm::mat4& mat);

  template<typename size_t_inserter>
  void queryBox(const gal::box3& box,
                size_t_inserter  inserter,
                eMeshElement     element) const
  {
    elementTree(element).queryBoxIntersects(box, inserter);
  };

  template<typename size_t_inserter>
  void querySphere(const glm::vec3& center,
                   float            radius,
                   size_t_inserter  inserter,
                   eMeshElement     element) const
  {
    elementTree(element).queryByDistance(center, radius, inserter);
  };

  glm::vec3 closestPoint(const glm::vec3& pt, float searchDist) const;
};

}  // namespace gal
