#pragma once
#include <Box.h>
#include <Util.h>
#include <numeric>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include <Mesh.h>

static constexpr float PLANE_DIST_TOL = 1e-10;

namespace gal {

struct IndexPair
{
  size_t p, q;

  bool operator==(const IndexPair&) const;
  bool operator!=(const IndexPair&) const;

  IndexPair(size_t i, size_t j);
  IndexPair();

  void   set(size_t, size_t);
  size_t hash() const;
  void   unset(size_t);
  bool   add(size_t);
  bool   contains(size_t) const;
};
}  // namespace gal

namespace std {

template<>
struct hash<gal::IndexPair>
{
  size_t operator()(const gal::IndexPair& ip) const noexcept { return ip.hash(); }
};

}  // namespace std

namespace gal {

class ConvexHull
{
public:
  struct Face
  {
    static const Face unset;

    size_t id;
    union
    {
      struct
      {
        size_t a, b, c;
      };
      std::array<size_t, 3> indices = {};
    };

    glm::vec3 normal = {};

    Face();
    Face(size_t i, size_t v1, size_t v2, size_t v3);

    bool           isValid();
    void           flip();
    gal::IndexPair edge(char edgeIndex) const;
    bool           containsVertex(size_t vertIndex) const;
  };

private:
  std::vector<glm::vec3> mPts;
  glm::vec3              mCenter = {};

  std::unordered_map<size_t, Face>                   mFaces;
  std::unordered_map<gal::IndexPair, gal::IndexPair> mEdgeFaceMap;
  std::unordered_set<size_t>                         mOutsidePts;

  void      compute();
  void      initOutside();
  void      setFace(Face& face);
  Face      popFace(size_t                        index,
                    std::array<gal::IndexPair, 3> edges,
                    std::array<Face, 3>           adjFaces);
  bool      faceVisible(const Face&, const glm::vec3&) const;
  float     facePlaneDistance(const Face&, const glm::vec3&) const;
  bool      getFarthestPt(const Face&, glm::vec3& pt, size_t& ptIndex) const;
  void      updateExteriorPt(const std::vector<Face>& newFaces,
                             const std::vector<Face>& poppedFaces);
  void      createInitialSimplex(size_t& faceIndex);
  bool      getFace(size_t id, Face& face) const;
  bool      getEdgeFaces(const gal::IndexPair& edge, gal::IndexPair& faces) const;
  glm::vec3 faceCenter(const Face& face) const;

public:
  template<typename vec3Iter>
  ConvexHull(vec3Iter vbegin, vec3Iter vend)
      : mPts(vbegin, vend)
  {
    initOutside();
    compute();
  };

  explicit ConvexHull(const std::vector<glm::vec3>& points);
  explicit ConvexHull(std::vector<glm::vec3>&& points);

  glm::vec3 getPt(size_t index) const;
  size_t    numFaces() const;
  void      copyFaces(int* faceIndices) const;

  TriMesh toMesh() const;
};

}  // namespace gal

namespace std {}  // namespace std
