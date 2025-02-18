#include <ConvexHull.h>

namespace gal {

bool gal::IndexPair::operator==(const gal::IndexPair& pair) const
{
  return (p == pair.p && q == pair.q) || (p == pair.q && q == pair.p);
}

bool gal::IndexPair::operator!=(const gal::IndexPair& pair) const
{
  return (p != pair.q && p != pair.p) || (q != pair.p && q != pair.q);
}

gal::IndexPair::IndexPair(size_t i, size_t j)
    : p(i)
    , q(j)
{}

gal::IndexPair::IndexPair()
    : p(-1)
    , q(-1)
{}

void gal::IndexPair::set(size_t i, size_t j)
{
  p = i;
  q = j;
}

size_t gal::IndexPair::hash() const
{
  return p + q + p * q;
}

void gal::IndexPair::unset(size_t i)
{
  if (p == i) {
    p = -1;
  }
  else if (q == i) {
    q = -1;
  }
}

bool gal::IndexPair::add(size_t i)
{
  if (p == -1) {
    p = i;
    return true;
  }
  else if (q == -1) {
    q = i;
    return true;
  }
  return false;
}

bool gal::IndexPair::contains(size_t i) const
{
  return (i != -1) && (i == p || i == q);
}

const ConvexHull::Face ConvexHull::Face::unset = Face(-1, -1, -1, -1);

ConvexHull::Face::Face()
    : id(-1)
    , a(-1)
    , b(-1)
    , c(-1)
{}

ConvexHull::Face::Face(size_t idVal, size_t v1, size_t v2, size_t v3)
    : id(idVal)
    , a(v1)
    , b(v2)
    , c(v3)
    , normal(vec3_unset)
{}

ConvexHull::ConvexHull(std::vector<glm::vec3>&& points)
    : mPts(std::move(points))
{
  initOutside();
  compute();
}

ConvexHull::ConvexHull(const std::vector<glm::vec3>& points)
    : mPts(points)
{
  initOutside();
  compute();
}

void ConvexHull::initOutside()
{
  size_t nPts = mPts.size();
  mOutsidePts.reserve(nPts);
  for (size_t i = 0; i < nPts; i++) {
    mOutsidePts.insert(i);
  }
}

bool ConvexHull::Face::isValid()
{
  return (id != -1 && a != -1 && b != -1 && c != -1) && (a != b && b != c && c != a);
}

void ConvexHull::Face::flip()
{
  std::swap(b, c);
  normal = -normal;
}

gal::IndexPair ConvexHull::Face::edge(char edgeIndex) const
{
  switch (edgeIndex) {
  case 0:
    return gal::IndexPair(a, b);
  case 1:
    return gal::IndexPair(b, c);
  case 2:
    return gal::IndexPair(c, a);
  default:
    throw "Invalid edge index.";
  }
}

bool ConvexHull::Face::containsVertex(size_t vi) const
{
  return vi == -1 && (vi == a || vi == b || vi == c);
}

glm::vec3 ConvexHull::getPt(size_t index) const
{
  return index < 0 || index > mPts.size() - 1 ? vec3_unset : mPts[index];
}

size_t ConvexHull::numFaces() const
{
  return mFaces.size();
}

void ConvexHull::copyFaces(int* faceIndices) const
{
  int i = 0;
  for (auto const& pair : mFaces) {
    faceIndices[i++] = (int)pair.second.a;
    faceIndices[i++] = (int)pair.second.b;
    faceIndices[i++] = (int)pair.second.c;
  }
}

void ConvexHull::compute()
{
  size_t curFaceId = 0;
  createInitialSimplex(curFaceId);
  std::queue<size_t> faceQ;
  for (const auto& f : mFaces) {
    faceQ.push(f.first);
  }

  Face                          curFace, pFace, newFace;
  std::array<Face, 3>           adjFaces;
  std::array<gal::IndexPair, 3> edges;
  glm::vec3                     farPt;
  std::queue<size_t>            popQ;
  std::vector<gal::IndexPair>   horizonEdges;
  std::vector<Face>             poppedFaces, newFaces;

  while (!faceQ.empty()) {
    size_t fi  = faceQ.front();
    size_t fpi = 0;
    faceQ.pop();
    if (!getFace(fi, curFace) || !getFarthestPt(curFace, farPt, fpi)) {
      continue;
    }
    popQ.push(fi);

    horizonEdges.clear();
    poppedFaces.clear();
    while (!popQ.empty()) {
      pFace = popFace(popQ.front(), edges, adjFaces);
      popQ.pop();

      if (!pFace.isValid()) {
        continue;
      }

      poppedFaces.push_back(pFace);

      for (size_t i = 0; i < 3; i++) {
        if (!adjFaces[i].isValid()) {
          continue;
        }
        if (faceVisible(adjFaces[i], farPt)) {
          popQ.push(adjFaces[i].id);
        }
        else {
          horizonEdges.push_back(edges[i]);
        }
      }
    }

    newFaces.clear();
    newFaces.reserve(horizonEdges.size());
    for (const gal::IndexPair& he : horizonEdges) {
      newFace = Face(curFaceId++, fpi, he.p, he.q);
      setFace(newFace);
      faceQ.push(newFace.id);
      newFaces.push_back(newFace);
    }

    updateExteriorPt(newFaces, poppedFaces);
  }
}

void ConvexHull::setFace(Face& face)
{
  if (!face.isValid())
    return;
  face.normal =
    glm::normalize(glm::cross(mPts[face.b] - mPts[face.a], mPts[face.c] - mPts[face.a]));
  if (faceVisible(face, mCenter)) {
    face.flip();
  }

  mFaces.insert_or_assign(face.id, face);

  for (char ei = 0; ei < 3; ei++) {
    if (!mEdgeFaceMap[face.edge(ei)].add(face.id)) {
      throw "Failed to add face to the edge map.";
    }
  }
}

ConvexHull::Face ConvexHull::popFace(size_t                        id,
                                     std::array<gal::IndexPair, 3> edges,
                                     std::array<Face, 3>           adjFaces)
{
  Face face;
  if (getFace(id, face)) {
    mFaces.erase(id);
    gal::IndexPair edge, fPair;
    size_t         adjFid = 0;
    for (char ei = 0; ei < 3; ei++) {
      edge      = face.edge(ei);
      edges[ei] = edge;
      if (!getEdgeFaces(edge, fPair) || !fPair.contains(id)) {
        adjFaces[ei] = Face::unset;
        continue;
      }
      fPair.unset(id);
      mEdgeFaceMap[edge] = fPair;
      adjFid             = fPair.p == -1 ? fPair.q : fPair.p;
      if (!getFace(adjFid, adjFaces[ei])) {
        adjFaces[ei] = Face::unset;
      }
    }
  }

  return face;
}

bool ConvexHull::faceVisible(const Face& face, const glm::vec3& pt) const
{
  return gal::utils::isValid(face.normal) ? facePlaneDistance(face, pt) > PLANE_DIST_TOL
                                          : false;
}

float ConvexHull::facePlaneDistance(const Face& face, const glm::vec3& pt) const
{
  return glm::dot(pt - mPts[face.a], face.normal);
}

bool ConvexHull::getFarthestPt(const Face& face, glm::vec3& pt, size_t& ptIndex) const
{
  ptIndex    = -1;
  pt         = vec3_unset;
  float dMax = PLANE_DIST_TOL, dist = 0.f;
  for (const size_t& i : mOutsidePts) {
    if (face.containsVertex(i)) {
      continue;
    }
    dist = facePlaneDistance(face, mPts[i]);
    if (dist > dMax) {
      dMax    = dist;
      ptIndex = i;
      pt      = mPts[i];
    }
  }

  return ptIndex != -1;
}

void ConvexHull::updateExteriorPt(const std::vector<Face>& newFaces,
                                  const std::vector<Face>& poppedFaces)
{
  bool                outside {};
  glm::vec3           testPt;
  std::vector<size_t> remove, check;
  for (const size_t& opi : mOutsidePts) {
    outside = false;
    testPt  = mPts[opi];
    for (const Face& face : poppedFaces) {
      if (face.containsVertex(opi)) {
        remove.push_back(opi);
        break;
      }
      if (faceVisible(face, testPt)) {
        outside = true;
        break;
      }
    }

    if (outside) {
      check.push_back(opi);
    }
  }

  for (const size_t& ci : check) {
    outside = false;
    testPt  = mPts[ci];
    for (const Face& newFace : newFaces) {
      if (faceVisible(newFace, testPt)) {
        outside = true;
        break;
      }
    }

    if (!outside) {
      remove.push_back(ci);
    }
  }

  for (const size_t& ri : remove) {
    mOutsidePts.erase(ri);
  }
}

void ConvexHull::createInitialSimplex(size_t& faceIndex)
{
  std::array<size_t, 4> best {};
  if (mPts.size() < 4) {
    throw "Failed to create the initial simplex";
  }
  else if (mPts.size() == 4) {
    for (size_t i = 0; i < 4; i++) {
      best[i] = i;
    }
  }
  else {
    std::array<float, 6> extremes {};
    for (size_t ei = 0; ei < 6; ei++) {
      extremes[ei] = ei % 2 == 0 ? FLT_MAX : -FLT_MAX;
    }
    std::array<size_t, 6> bounds {};
    for (size_t i = 0; i < 6; i++) {
      bounds[i] = (size_t)(-1);
    }
    std::array<float, 6> coords {};
    for (size_t pi = 0; pi < mPts.size(); pi++) {
      std::copy_n(&mPts[pi].x, 3, coords.data());
      for (size_t ei = 0; ei < 6; ei++) {
        if ((ei % 2 == 0 && extremes[ei] > coords[ei / 2]) ||
            (ei % 2 == 1 && extremes[ei] < coords[ei / 2])) {
          extremes[ei] = coords[ei / 2];
          bounds[ei]   = pi;
        }
      }
    }

    glm::vec3 pt;
    float     maxD = -FLT_MAX, dist = 0.f;
    for (size_t i = 0; i < 6; i++) {
      pt = mPts[bounds[i]];
      for (size_t j = i + 1; j < 6; j++) {
        dist = glm::length2(pt - mPts[bounds[j]]);
        if (dist > maxD) {
          best[0] = bounds[i];
          best[1] = bounds[j];
          maxD    = dist;
        }
      }
    }

    if (maxD <= 0) {
      throw "Failed to create the initial simplex";
    }

    maxD           = -FLT_MAX;
    glm::vec3 ref  = mPts[best[0]];
    glm::vec3 uDir = glm::normalize(mPts[best[1]] - ref);
    for (size_t pi = 0; pi < mPts.size(); pi++) {
      dist = glm::length2((mPts[pi] - ref) - uDir * glm::dot(uDir, (mPts[pi] - ref)));
      if (dist > maxD) {
        best[2] = pi;
        maxD    = dist;
      }
    }

    if (maxD <= 0) {
      throw "Failed to create the initial simplex";
    }

    maxD = -FLT_MAX;
    uDir = glm::normalize(glm::cross(mPts[best[1]] - ref, mPts[best[2]] - ref));
    for (size_t pi = 0; pi < mPts.size(); pi++) {
      dist = std::abs(glm::dot(uDir, (mPts[pi] - ref)));
      if (dist > maxD) {
        best[3] = pi;
        maxD    = dist;
      }
    }

    if (maxD <= 0) {
      throw "Failed to create the initial simplex";
    }
  }

  std::array<Face, 4> simplex;
  simplex[0] = Face(faceIndex++, best[0], best[1], best[2]);
  simplex[1] = Face(faceIndex++, best[0], best[2], best[3]);
  simplex[2] = Face(faceIndex++, best[1], best[2], best[3]);
  simplex[3] = Face(faceIndex++, best[0], best[1], best[3]);

  mCenter = vec3_zero;
  for (size_t i = 0; i < 4; i++) {
    mCenter += mPts[best[i]];
  }
  mCenter /= 4;

  for (size_t i = 0; i < 4; i++) {
    if (!simplex[i].isValid()) {
      continue;
    }
    setFace(simplex[i]);
  }

  std::vector<size_t> removePts;
  for (const size_t& opi : mOutsidePts) {
    bool outside = false;
    for (size_t i = 0; i < 4; i++) {
      if (simplex[i].containsVertex(opi)) {
        removePts.push_back(opi);
        break;
      }
      if (faceVisible(simplex[i], getPt(opi))) {
        outside = true;
        break;
      }
    }

    if (!outside) {
      removePts.push_back(opi);
    }
  }

  for (const size_t& ri : removePts) {
    mOutsidePts.erase(ri);
  }
}

bool ConvexHull::getFace(size_t id, Face& face) const
{
  auto match = mFaces.find(id);
  if (match != mFaces.end()) {
    face = match->second;
    return true;
  }
  return false;
}

bool ConvexHull::getEdgeFaces(const gal::IndexPair& edge, gal::IndexPair& faces) const
{
  auto match = mEdgeFaceMap.find(edge);
  if (match != mEdgeFaceMap.end()) {
    faces = match->second;
    return true;
  }
  return false;
}

glm::vec3 ConvexHull::faceCenter(const Face& face) const
{
  return (mPts[face.a] + mPts[face.b] + mPts[face.c]) / 3.0f;
}

TriMesh ConvexHull::toMesh() const
{
  TriMesh            mesh;
  std::vector<VertH> vmap(mPts.size());
  size_t             nedges = mFaces.size() * 3 / 2;
  mesh.reserve(nedges, nedges, mFaces.size());
  for (const auto& pair : mFaces) {
    const auto&          face = pair.second;
    std::array<VertH, 3> fvs;
    for (uint8_t i = 0; i < 3; i++) {
      int   vi = int(face.indices[i]);
      auto& v  = vmap[vi];
      if (!v.is_valid()) {
        v = gal::handle<VertH>(mesh.add_vertex(mPts[vi]));
      }
      fvs[i] = v;
    }
    mesh.add_face(fvs.data(), fvs.size());
  }
  return mesh;
}

}  // namespace gal
