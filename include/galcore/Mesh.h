#pragma once
#include <galcore/Box.h>
#include <galcore/Plane.h>
#include <galcore/RTree.h>
#include <galcore/Sphere.h>
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
    explicit Face(size_t const indices[3]);

    void     flip();
    EdgeType edge(uint8_t edgeIndex) const;
    bool     containsVertex(size_t vertIndex) const;
    bool     isDegenerate() const;
  };

  struct EdgeTriplet
  {
    size_t a = SIZE_MAX, b = SIZE_MAX, c = SIZE_MAX;

    EdgeTriplet() = default;
    explicit EdgeTriplet(size_t const (&indices)[3]);
    EdgeTriplet(size_t, size_t, size_t);

    void set(size_t, size_t, size_t);
    void set(size_t);
  };

private:
  using ConstVertIter = std::vector<glm::vec3>::const_iterator;
  using ConstFaceIter = std::vector<Face>::const_iterator;

  std::vector<glm::vec3>         mVertices;
  std::vector<Face>              mFaces;
  mutable std::vector<glm::vec3> mVertexColors;

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
  Mesh() = default;
  Mesh(const Mesh& other);
  Mesh(Mesh&& other);
  Mesh(const glm::vec3* verts, size_t nVerts, const Face* faces, size_t nFaces);
  Mesh(const std::vector<glm::vec3>& verts, const std::vector<Face>& faces);
  Mesh(std::vector<glm::vec3>&& verts, std::vector<Face>&& faces);
  Mesh(const float*  vertCoords,
       size_t        nVerts,
       const size_t* faceVertIndices,
       size_t        nFaces);

  Mesh& operator=(const Mesh& mesh);
  Mesh& operator=(Mesh&& mesh);

  size_t                        numVertices() const noexcept;
  size_t                        numFaces() const noexcept;
  size_t                        numEdges() const noexcept;
  glm::vec3                     vertex(size_t vi) const;
  Face                          face(size_t fi) const;
  glm::vec3                     vertexNormal(size_t vi) const;
  const glm::vec3&              faceNormal(size_t fi) const;
  const std::vector<glm::vec3>& vertices() const;
  const std::vector<Face>&      faces() const;
  Mesh::ConstVertIter           vertexCBegin() const;
  Mesh::ConstVertIter           vertexCEnd() const;
  Mesh::ConstFaceIter           faceCBegin() const;
  Mesh::ConstFaceIter           faceCEnd() const;
  gal::Box3                     bounds() const;
  float                         faceArea(size_t fi) const;
  float                         area() const;
  gal::Box3                     faceBounds(size_t fi) const;
  void                          setVertexColors(std::vector<glm::vec3> colors);
  const std::vector<glm::vec3>& vertexColors() const;
  const glm::vec3&              vertexColor(size_t vi) const;
  void                          vertexColor(const glm::vec3& color, size_t vi);

  template<typename TColorIter>
  void setVertexColors(TColorIter begin, TColorIter end)
  {
    mVertexColors.resize(numVertices(), glm::vec3(0.f, 0.f, 0.f));
    auto dst = mVertexColors.begin();
    while (begin != end && dst != mVertexColors.end()) {
      *(dst++) = *(begin++);
    }
  }

  float     volume() const;
  bool      isSolid() const;
  glm::vec3 centroid() const;
  glm::vec3 centroid(const eMeshCentroidType centroid_type) const;

  bool contains(const glm::vec3& pt) const;

  /**
   * @brief Gets the result of clipping this mesh with the given plane. This mesh instance
   * not modified.
   *
   * @param plane The plane to clip with.
   * @return Mesh Clipped mesh.
   */
  Mesh clippedWithPlane(const Plane& plane) const;

  void transform(const glm::mat4& mat);

  template<typename size_t_inserter>
  void queryBox(const gal::Box3& box,
                size_t_inserter  inserter,
                eMeshElement     element) const
  {
    elementTree(element).queryBoxIntersects(box, inserter);
  };

  template<typename size_t_inserter>
  void querySphere(const gal::Sphere& sphere,
                   size_t_inserter    inserter,
                   eMeshElement       element) const
  {
    elementTree(element).queryByDistance(sphere.center, sphere.radius, inserter);
  };

  Mesh extractFaces(const std::vector<size_t>& faces) const;

  glm::vec3 closestPoint(const glm::vec3& pt, float searchDist) const;
};

/**
 * @brief Creates a rectangular mesh surface with aligned quad faces.
 *
 * @param plane Plane
 * @param box 2D box representing the interval of the rectangle in the plane.
 * @param edgeLength The approximate size of the quad.
 * @return gal::Mesh Surface mesh.
 */
gal::Mesh createRectangularMesh(const gal::Plane& plane,
                                const gal::Box2&  box,
                                float             edgeLength);

template<>
struct IsValueType<Mesh::Face> : public std::true_type
{
};

template<>
struct Serial<Mesh> : public std::true_type
{
  static Mesh deserialize(Bytes& bytes)
  {
    std::vector<glm::vec3>  verts;
    std::vector<glm::vec3>  vertexColors;
    std::vector<Mesh::Face> faces;
    bytes >> verts >> faces >> vertexColors;
    Mesh mesh(verts, faces);
    mesh.setVertexColors(std::move(vertexColors));
    return mesh;
  }
  static Bytes serialize(const Mesh& msh)
  {
    Bytes bytes;
    bytes << msh.vertices() << msh.faces();
    return bytes;
  }
};

}  // namespace gal
