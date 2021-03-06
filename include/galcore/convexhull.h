#pragma once
#include "base.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>

constexpr double PLANE_DIST_TOL = 1e-10;

struct hullFace {
  static const hullFace unset;

  size_t id;
  size_t a, b, c;
  vec3 normal;

  hullFace();
  hullFace(size_t i, size_t v1, size_t v2, size_t v3);

  bool is_valid();
  void flip();
  indexPair edge(char edgeIndex) const;
  bool containsVertex(size_t vertIndex) const;
};

class convexHull {
private:
  std::vector<vec3> mPts;
  size_t mNumPts;
  vec3 m_center;

  std::unordered_map<size_t, hullFace, customSizeTHash, std::equal_to<size_t>>
      mFaces;
  std::unordered_map<indexPair, indexPair, indexPairHash,
                     std::equal_to<indexPair>>
      mEdgeFaceMap;
  std::unordered_set<size_t, customSizeTHash, std::equal_to<size_t>>
      mOutsidePts;

  void compute();
  void setFace(hullFace &face);
  hullFace popFace(size_t index, indexPair edges[3], hullFace adjFaces[3]);
  bool faceVisible(const hullFace &, const vec3 &) const;
  double facePlaneDistance(const hullFace &, const vec3 &) const;
  bool getFarthestPt(const hullFace &, vec3 &pt, size_t &ptIndex) const;
  void updateExteriorPt(const std::vector<hullFace> &newFaces,
                        const std::vector<hullFace> &poppedFaces);
  void createInitialSimplex(size_t &faceIndex);
  bool getFace(size_t id, hullFace &face) const;
  bool getEdgeFaces(const indexPair &edge, indexPair &faces) const;
  vec3 faceCenter(const hullFace &face) const;

public:
  convexHull(double *coords, size_t nPts);

  vec3 getPt(size_t index) const;
  size_t numFaces() const;
  void copyFaces(int *faceIndices) const;
};
