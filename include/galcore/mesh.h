#pragma once
#include "base.h"
#include "rtree.h"
#include <limits>
#include <unordered_map>

typedef indexPair edge_type;
typedef indexPairHash edge_type_hash;

struct meshFace {
  static const meshFace unset;
  union {
    struct {
      size_t a, b, c;
    };
    size_t indices[3];
  };

  meshFace();
  meshFace(size_t v1, size_t v2, size_t v3);
  meshFace(size_t const indices[3]);

  void flip();
  edge_type edge(uint8_t edgeIndex) const;
  bool containsVertex(size_t vertIndex) const;
  bool is_degenerate() const;
};

struct faceEdges {
  size_t a = SIZE_MAX, b = SIZE_MAX, c = SIZE_MAX;

  faceEdges() = default;
  faceEdges(size_t const indices[3]);
  faceEdges(size_t, size_t, size_t);

  void set(size_t, size_t, size_t);
  void set(size_t);
};

enum class meshCentroidType { vertex_based = 0, area_based, volume_based };

enum class meshElement { vertex, face };

class mesh {
  typedef std::vector<vec3>::const_iterator const_vertex_iterator;
  typedef std::vector<meshFace>::const_iterator const_face_iterator;

private:
  std::vector<vec3> mVertices;
  std::vector<meshFace> mFaces;

  /*Maps vertex indices to indices of connected faces.*/
  std::vector<std::vector<size_t>> mVertFaces;
  /*Maps the vertex indices to the indices of connected edges.*/
  std::vector<std::vector<size_t>> mVertEdges;
  /*Maps the vertex-index-pair to the index of the edge connecting those
   * vertices.*/
  std::unordered_map<edge_type, size_t, edge_type_hash> mEdgeIndexMap;
  /*Maps the edge index to the pair of connected vertex indices.*/
  std::vector<edge_type> mEdges;
  /*Maps the edge index to the indices of the connected faces.*/
  std::vector<std::vector<size_t>> mEdgeFaces;
  /*Maps the face index to the indices of the 3 edges connected to that face.*/
  std::vector<faceEdges> mFaceEdges;

  std::vector<vec3> mVertexNormals;
  std::vector<vec3> mFaceNormals;
  bool mIsSolid;
  rtree3d m_faceTree;
  rtree3d mVertexTree;

  void computeCache();
  void computeTopology();
  void computeRTrees();
  void computeNormals();
  void addEdge(const meshFace &, size_t fi, uint8_t, size_t &);
  void addEdges(const meshFace &, size_t fi);
  double faceArea(const meshFace &f) const;
  void getFaceCenter(const meshFace &f, vec3 &center) const;
  void checkSolid();

  vec3 areaCentroid() const;
  vec3 volumeCentroid() const;
  const rtree3d &element_tree(meshElement element) const;

  void faceClosestPt(size_t faceIndex, const vec3 &pt, vec3 &closePt,
                     double &bestSqDist) const;

public:
  mesh(const mesh &other);
  mesh(const vec3 *verts, size_t nVerts, const meshFace *faces, size_t nFaces);
  mesh(const double *vertCoords, size_t nVerts, const size_t *faceVertIndices,
       size_t nFaces);

  size_t num_vertices() const noexcept;
  size_t numFaces() const noexcept;
  vec3 vertex(size_t vi) const;
  meshFace face(size_t fi) const;
  vec3 vertex_normal(size_t vi) const;
  const vec3 &face_normal(size_t fi) const;
  mesh::const_vertex_iterator vertex_cbegin() const;
  mesh::const_vertex_iterator vertex_cend() const;
  mesh::const_face_iterator face_cbegin() const;
  mesh::const_face_iterator face_cend() const;

  box3 bounds() const;
  double faceArea(size_t fi) const;
  double area() const;
  box3 face_bounds(size_t fi) const;

  double volume() const;
  bool is_solid() const;
  vec3 centroid() const;
  vec3 centroid(const meshCentroidType centroid_type) const;

  bool contains(const vec3 &pt) const;

  mesh *clipped_with_plane(const vec3 &pt, const vec3 &norm) const;

  template <typename size_t_inserter>
  void query_box(const box3 &box, size_t_inserter inserter,
                 meshElement element) const {
    element_tree(element).query_box_intersects(box, inserter);
  };

  template <typename size_t_inserter>
  void query_sphere(const vec3 &center, double radius, size_t_inserter inserter,
                    meshElement element) const {
    element_tree(element).query_by_distance(center, radius, inserter);
  };

  vec3 closest_point(const vec3 &pt, double searchDist) const;
};
