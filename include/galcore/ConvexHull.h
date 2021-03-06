#pragma once
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include "base.h"

static constexpr float PLANE_DIST_TOL = 1e-10;

class ConvexHull
{
public:
  struct Face
  {
    static const Face unset;

    size_t    id;
    size_t    a, b, c;
    glm::vec3 normal;

    Face();
    Face(size_t i, size_t v1, size_t v2, size_t v3);

    bool      is_valid();
    void      flip();
    IndexPair edge(char edgeIndex) const;
    bool      containsVertex(size_t vertIndex) const;
  };

private:
  std::vector<glm::vec3> mPts;
  size_t                 mNumPts;
  glm::vec3              m_center;

  std::unordered_map<size_t, Face, CustomSizeTHash, std::equal_to<size_t>> mFaces;
  std::unordered_map<IndexPair, IndexPair, IndexPairHash, std::equal_to<IndexPair>>
                                                                     mEdgeFaceMap;
  std::unordered_set<size_t, CustomSizeTHash, std::equal_to<size_t>> mOutsidePts;

  void      compute();
  void      setFace(Face& face);
  Face      popFace(size_t index, IndexPair edges[3], Face adjFaces[3]);
  bool      faceVisible(const Face&, const glm::vec3&) const;
  float     facePlaneDistance(const Face&, const glm::vec3&) const;
  bool      getFarthestPt(const Face&, glm::vec3& pt, size_t& ptIndex) const;
  void      updateExteriorPt(const std::vector<Face>& newFaces,
                             const std::vector<Face>& poppedFaces);
  void      createInitialSimplex(size_t& faceIndex);
  bool      getFace(size_t id, Face& face) const;
  bool      getEdgeFaces(const IndexPair& edge, IndexPair& faces) const;
  glm::vec3 faceCenter(const Face& face) const;

public:
  ConvexHull(float* coords, size_t nPts);

  glm::vec3 getPt(size_t index) const;
  size_t    numFaces() const;
  void      copyFaces(int* faceIndices) const;
};
