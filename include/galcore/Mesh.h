#pragma once
#include "RTree.h"
#include "base.h"
#include <limits>
#include <unordered_map>

using EdgeType = IndexPair;
using EdgeTypeHash = IndexPairHash;

enum class eMeshCentroidType { vertexBased = 0, areaBased, volumeBased };

enum class eMeshElement { vertex, face };

class Mesh {
public:
  struct Face {
    static const Face unset;
    union {
      struct {
        size_t a, b, c;
      };
      size_t indices[3];
    };

    Face();
    Face(size_t v1, size_t v2, size_t v3);
    Face(size_t const indices[3]);

    void flip();
    EdgeType edge(uint8_t edgeIndex) const;
    bool containsVertex(size_t vertIndex) const;
    bool isDegenerate() const;
  };

  struct EdgeTriplet {
    size_t a = SIZE_MAX, b = SIZE_MAX, c = SIZE_MAX;

    EdgeTriplet() = default;
    EdgeTriplet(size_t const (&indices)[3]);
    EdgeTriplet(size_t, size_t, size_t);

    void set(size_t, size_t, size_t);
    void set(size_t);
  };

private:
  using ConstVertIter = std::vector<vec3>::const_iterator;
  using ConstFaceIter = std::vector<Face>::const_iterator;

  std::vector<vec3> mVertices;
  std::vector<Face> mFaces;

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

  std::vector<vec3> mVertexNormals;
  std::vector<vec3> mFaceNormals;
  bool mIsSolid;
  RTree3d mFaceTree;
  RTree3d mVertexTree;

  void computeCache();
  void computeTopology();
  void computeRTrees();
  void computeNormals();
  void addEdge(const Face &, size_t fi, uint8_t, size_t &);
  void addEdges(const Face &, size_t fi);
  double faceArea(const Face &f) const;
  void getFaceCenter(const Face &f, vec3 &center) const;
  void checkSolid();

  vec3 areaCentroid() const;
  vec3 volumeCentroid() const;
  const RTree3d &elementTree(eMeshElement element) const;

  void faceClosestPt(size_t faceIndex, const vec3 &pt, vec3 &closePt,
                     double &bestSqDist) const;

public:
  Mesh(const Mesh &other);
  Mesh(const vec3 *verts, size_t nVerts, const Face *faces, size_t nFaces);
  Mesh(const double *vertCoords, size_t nVerts, const size_t *faceVertIndices,
       size_t nFaces);

  size_t numVertices() const noexcept;
  size_t numFaces() const noexcept;
  vec3 vertex(size_t vi) const;
  Face face(size_t fi) const;
  vec3 vertexNormal(size_t vi) const;
  const vec3 &faceNormal(size_t fi) const;
  Mesh::ConstVertIter vertexCBegin() const;
  Mesh::ConstVertIter vertexCEnd() const;
  Mesh::ConstFaceIter faceCBegin() const;
  Mesh::ConstFaceIter faceCEnd() const;

  box3 bounds() const;
  double faceArea(size_t fi) const;
  double area() const;
  box3 faceBounds(size_t fi) const;

  double volume() const;
  bool isSolid() const;
  vec3 centroid() const;
  vec3 centroid(const eMeshCentroidType centroid_type) const;

  bool contains(const vec3 &pt) const;

  Mesh *clippedWithPlane(const vec3 &pt, const vec3 &norm) const;

  template <typename size_t_inserter>
  void queryBox(const box3 &box, size_t_inserter inserter,
                eMeshElement element) const {
    elementTree(element).queryBoxIntersects(box, inserter);
  };

  template <typename size_t_inserter>
  void querySphere(const vec3 &center, double radius, size_t_inserter inserter,
                   eMeshElement element) const {
    elementTree(element).queryByDistance(center, radius, inserter);
  };

  vec3 closestPoint(const vec3 &pt, double searchDist) const;
};
