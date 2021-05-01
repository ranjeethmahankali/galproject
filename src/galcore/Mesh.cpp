#include "galcore/Mesh.h"
#define _USE_MATH_DEFINES
#include <galcore/DebugProfile.h>
#include <galcore/ObjLoader.h>
#include <math.h>
#include <array>
#include <numeric>

static constexpr uint8_t                               X = UINT8_MAX;
static constexpr std::array<std::array<uint8_t, 6>, 8> s_clipTriTable {{
  {X, X, X, X, X, X},
  {0, 3, 5, X, X, X},
  {3, 1, 4, X, X, X},
  {0, 1, 5, 1, 4, 5},
  {4, 2, 5, X, X, X},
  {0, 3, 4, 0, 4, 2},
  {1, 5, 3, 1, 2, 5},
  {0, 1, 2, X, X, X},
}};

static constexpr std::array<uint8_t, 8> s_clipVertCountTable {0, 3, 3, 6, 3, 6, 6, 3};

namespace gal {

const Mesh::Face Mesh::Face::unset = Face(-1, -1, -1);

Mesh::Face::Face()
    : a(SIZE_MAX)
    , b(SIZE_MAX)
    , c(SIZE_MAX)
{}

Mesh::Face::Face(size_t v1, size_t v2, size_t v3)
    : a(v1)
    , b(v2)
    , c(v3)
{}

Mesh::Face::Face(size_t const indices[3])
    : Face(indices[0], indices[1], indices[2])
{}

void Mesh::Face::flip()
{
  size_t temp = c;
  c           = b;
  b           = temp;
}

EdgeType Mesh::Face::edge(uint8_t edgeIndex) const
{
  switch (edgeIndex) {
  case 0:
    return gal::IndexPair(a, b);
  case 1:
    return gal::IndexPair(b, c);
  case 2:
    return gal::IndexPair(c, a);
  default:
    throw edgeIndex;
  }
}

bool Mesh::Face::containsVertex(size_t vertIndex) const
{
  return a == vertIndex || b == vertIndex || c == vertIndex;
}

bool Mesh::Face::isDegenerate() const
{
  return a == b || b == c || c == a;
}

void Mesh::computeCache()
{
  computeRTrees();
  computeTopology();
  computeNormals();
  checkSolid();
}

void Mesh::computeTopology()
{
  mVertEdges.clear();
  mVertFaces.clear();
  mFaceEdges.clear();
  mVertEdges.resize(numVertices());
  mVertFaces.resize(numVertices());
  mFaceEdges.resize(numFaces());

  size_t nVertices = numVertices();
  size_t curEi     = 0;
  for (size_t fi = 0; fi < mFaces.size(); fi++) {
    Face f = mFaces[fi];
    mVertFaces[f.a].push_back(fi);
    mVertFaces[f.b].push_back(fi);
    mVertFaces[f.c].push_back(fi);

    addEdges(f, fi);
  }
}

void Mesh::computeRTrees()
{
  mFaceTree.clear();
  for (size_t fi = 0; fi < mFaces.size(); fi++) {
    mFaceTree.insert(faceBounds(fi), fi);
  }

  mVertexTree.clear();
  size_t vi = 0;
  for (const glm::vec3& v : mVertices) {
    mVertexTree.insert(Box3(v), vi++);
  }
}

void Mesh::computeNormals()
{
  mFaceNormals.clear();
  mFaceNormals.reserve(mFaces.size());
  for (ConstFaceIter fi = faceCBegin(); fi != faceCEnd(); fi++) {
    const glm::vec3& a = mVertices.at(fi->a);
    const glm::vec3& b = mVertices.at(fi->b);
    const glm::vec3& c = mVertices.at(fi->c);
    mFaceNormals.push_back(glm::normalize(glm::cross(b - a, c - a)));
  }

  mVertexNormals.clear();
  mVertexNormals.resize(mVertices.size());
  std::vector<glm::vec3> faceNormals;
  for (size_t vi = 0; vi < mVertices.size(); vi++) {
    const auto& faces = mVertFaces.at(vi);
    faceNormals.clear();
    faceNormals.reserve(faces.size());
    std::transform(faces.cbegin(),
                   faces.cend(),
                   std::back_inserter(faceNormals),
                   [this](const size_t fi) { return mFaceNormals[fi]; });
    mVertexNormals[vi] =
      glm::normalize(utils::average(faceNormals.cbegin(), faceNormals.cend()));
  }
}

void Mesh::addEdge(const Face& f, size_t fi, uint8_t fei, size_t& newEi)
{
  EdgeType e      = f.edge(fei);
  auto     eMatch = mEdgeIndexMap.find(e);
  if (eMatch == mEdgeIndexMap.end()) {
    newEi = mEdges.size();
    mEdgeIndexMap.emplace(e, newEi);
    mEdges.push_back(e);
    mEdgeFaces.emplace_back();

    mVertEdges[e.p].push_back(newEi);
    mVertEdges[e.q].push_back(newEi);
  }
  else {
    newEi = eMatch->second;
  }

  mEdgeFaces[newEi].push_back(fi);
}

void Mesh::addEdges(const Face& f, size_t fi)
{
  size_t indices[3];
  for (uint8_t fei = 0; fei < 3; fei++) {
    addEdge(f, fi, fei, indices[fei]);
  }
  mFaceEdges[fi] = EdgeTriplet(indices);
}

float Mesh::faceArea(const Face& f) const
{
  const glm::vec3& a = vertex(f.a);
  return glm::length(glm::cross(vertex(f.b) - a, vertex(f.c) - a)) * 0.5f;
}

void Mesh::getFaceCenter(const Face& f, glm::vec3& center) const
{
  center = (mVertices[f.a] + mVertices[f.b] + mVertices[f.c]) / 3.0f;
}

void Mesh::checkSolid()
{
  for (const auto& faces : mEdgeFaces) {
    if (faces.size() != 2) {
      mIsSolid = false;
      return;
    }
  }
  mIsSolid = true;
}

glm::vec3 Mesh::areaCentroid() const
{
  std::vector<glm::vec3> centers;
  centers.reserve(numFaces());
  std::vector<float> areas;
  areas.reserve(numFaces());

  for (ConstFaceIter fIter = faceCBegin(); fIter != faceCEnd(); fIter++) {
    glm::vec3   center;
    const Face& f = *fIter;
    getFaceCenter(f, center);
    centers.push_back(center);
    areas.push_back(faceArea(f));
  }

  return utils::weightedAverage(centers.cbegin(), centers.cend(), areas.cbegin());
}

glm::vec3 Mesh::volumeCentroid() const
{
  glm::vec3              refPt = bounds().center();
  std::vector<glm::vec3> joinVecs;
  joinVecs.reserve(numVertices());
  std::transform(vertexCBegin(),
                 vertexCEnd(),
                 std::back_inserter(joinVecs),
                 [&refPt](const glm::vec3& vert) { return vert - refPt; });

  std::vector<glm::vec3> centers;
  centers.reserve(numFaces());
  std::vector<float> volumes;
  volumes.reserve(numFaces());

  for (size_t fi = 0; fi < mFaces.size(); fi++) {
    Face      f = mFaces[fi];
    glm::vec3 a = joinVecs[f.a];
    glm::vec3 b = joinVecs[f.b];
    glm::vec3 c = joinVecs[f.c];

    float volume = std::abs(glm::dot(glm::cross(a, b), c)) / 6.0f;
    if (glm::dot(a, faceNormal(fi)) < 0.0f) {
      volume *= -1;
    }
    volumes.push_back(volume);

    a = mVertices[f.a];
    b = mVertices[f.b];
    c = mVertices[f.c];
    centers.push_back((a + b + c + refPt) * 0.25f);
  }

  return utils::weightedAverage(centers.cbegin(), centers.cend(), volumes.cbegin());
}

const RTree3d& Mesh::elementTree(eMeshElement element) const
{
  switch (element) {
  case eMeshElement::face:
    return mFaceTree;
  case eMeshElement::vertex:
    return mVertexTree;
  default:
    throw "Invalid element type";
  }
}

void Mesh::faceClosestPt(size_t           faceIndex,
                         const glm::vec3& pt,
                         glm::vec3&       closePt,
                         float&           bestSqDist) const
{
  const Face&      face       = mFaces.at(faceIndex);
  const glm::vec3& va         = mVertices.at(face.a);
  const glm::vec3& fnorm      = faceNormal(faceIndex);
  glm::vec3        projection = fnorm * glm::dot((va - pt), fnorm);

  float planeDistSq = glm::length2(projection);
  if (planeDistSq > bestSqDist)
    return;

  glm::vec3 projected = pt + projection;

  uint8_t nOutside = 0;
  for (uint8_t i = 0; i < 3; i++) {
    const glm::vec3& v1 = mVertices.at(face.indices[i]);
    const glm::vec3& v2 = mVertices.at(face.indices[(i + 1) % 3]);
    bool outside = glm::dot(glm::cross(v1 - projected, v2 - projected), fnorm) < 0.0f;
    if (outside) {
      nOutside++;
      glm::vec3 ln = v2 - v1;
      float     param =
        std::clamp(glm::dot(ln, projected - v1) / glm::length2(ln), 0.0f, 1.0f);
      glm::vec3 cpt    = v2 * param + v1 * (1.0f - param);
      float     distSq = glm::length2(cpt - pt);
      if (distSq < bestSqDist) {
        closePt    = cpt;
        bestSqDist = distSq;
      }
    }

    if (nOutside > 1)
      break;
  }

  if (nOutside == 0) {
    closePt    = projected;
    bestSqDist = planeDistSq;
  }
}

Mesh::Mesh(const Mesh& other)
    : Mesh(other.mVertices.data(),
           other.numVertices(),
           other.mFaces.data(),
           other.numFaces())
{}

Mesh::Mesh(const std::vector<glm::vec3>& verts, const std::vector<Face>& faces)
    : Mesh(verts.data(), verts.size(), faces.data(), faces.size()) {};

Mesh::Mesh(std::vector<glm::vec3>&& verts, std::vector<Face>&& faces)
    : mVertices(std::move(verts))
    , mFaces(std::move(faces))
{
  computeCache();
}

Mesh::Mesh(const glm::vec3* verts, size_t nVerts, const Face* faces, size_t nFaces)
{
  mVertices.reserve(nVerts);
  std::copy(verts, verts + nVerts, std::back_inserter(mVertices));
  mFaces.reserve(nFaces);
  std::copy(faces, faces + nFaces, std::back_inserter(mFaces));
  computeCache();
}

Mesh::Mesh(const float*  vertCoords,
           size_t        nVerts,
           const size_t* faceVertIndices,
           size_t        nFaces)
{
  mVertices.reserve(nVerts);
  size_t nFlat = nVerts * 3;
  size_t i     = 0;
  while (i < nFlat) {
    float x = vertCoords[i++];
    float y = vertCoords[i++];
    float z = vertCoords[i++];
    mVertices.emplace_back(x, y, z);
  }

  mFaces.reserve(nFaces);
  nFlat = nFaces * 3;
  i     = 0;
  while (i < nFlat) {
    size_t a = faceVertIndices[i++];
    size_t b = faceVertIndices[i++];
    size_t c = faceVertIndices[i++];
    mFaces.emplace_back(a, b, c);
  }

  computeCache();
}

size_t Mesh::numVertices() const noexcept
{
  return mVertices.size();
}

size_t Mesh::numFaces() const noexcept
{
  return mFaces.size();
}

glm::vec3 Mesh::vertex(size_t vi) const
{
  return vi < numVertices() ? mVertices[vi] : vec3_unset;
}

Mesh::Face Mesh::face(size_t fi) const
{
  return fi < numFaces() ? mFaces[fi] : Face::unset;
}

glm::vec3 Mesh::vertexNormal(size_t vi) const
{
  return vi < numVertices() ? mVertexNormals[vi] : vec3_unset;
}

const glm::vec3& Mesh::faceNormal(size_t fi) const
{
  return fi < numFaces() ? mFaceNormals.at(fi) : vec3_unset;
}

Mesh::ConstVertIter Mesh::vertexCBegin() const
{
  return mVertices.cbegin();
}

Mesh::ConstVertIter Mesh::vertexCEnd() const
{
  return mVertices.cend();
}

Mesh::ConstFaceIter Mesh::faceCBegin() const
{
  return mFaces.cbegin();
}

Mesh::ConstFaceIter Mesh::faceCEnd() const
{
  return mFaces.cend();
}

Box3 Mesh::bounds() const
{
  Box3 b;
  for (const glm::vec3& v : mVertices) {
    b.inflate(v);
  }
  return b;
}

float Mesh::faceArea(size_t fi) const
{
  return faceArea(face(fi));
}

float Mesh::area() const
{
  float sum = 0;
  for (const Face& f : mFaces) {
    sum += faceArea(f);
  }
  return sum;
}

Box3 Mesh::faceBounds(size_t fi) const
{
  Box3 b;
  Face f = mFaces[fi];
  b.inflate(mVertices[f.a]);
  b.inflate(mVertices[f.b]);
  b.inflate(mVertices[f.c]);
  return b;
}

float Mesh::volume() const
{
  if (!isSolid())
    return 0.0;

  glm::vec3              refPt = bounds().center();
  std::vector<glm::vec3> joinVectors;
  joinVectors.reserve(numVertices());
  std::transform(vertexCBegin(),
                 vertexCEnd(),
                 std::back_inserter(joinVectors),
                 [&refPt](const glm::vec3 vert) { return vert - refPt; });

  float total = 0.0;
  for (size_t fi = 0; fi < mFaces.size(); fi++) {
    Face      face      = mFaces[fi];
    glm::vec3 a         = joinVectors[face.a];
    glm::vec3 b         = joinVectors[face.b];
    glm::vec3 c         = joinVectors[face.c];
    float     tetVolume = std::abs(glm::dot(glm::cross(a, b), c) / 6.0f);

    if (glm::dot(a, faceNormal(fi)) > 0.0f)
      total += tetVolume;
    else
      total -= tetVolume;
  }

  return total;
}

bool Mesh::isSolid() const
{
  return mIsSolid;
}

glm::vec3 Mesh::centroid() const
{
  GALSCOPE(__func__);
  return centroid(eMeshCentroidType::vertexBased);
}

glm::vec3 Mesh::centroid(const eMeshCentroidType centroid_type) const
{
  GALSCOPE(__func__);
  switch (centroid_type) {
  case eMeshCentroidType::vertexBased:
    return utils::average(vertexCBegin(), vertexCEnd());
  case eMeshCentroidType::areaBased:
    return areaCentroid();
  case eMeshCentroidType::volumeBased:
    return volumeCentroid();
  default:
    return centroid(eMeshCentroidType::vertexBased);
  }
}

bool Mesh::contains(const glm::vec3& pt) const
{
  static const auto comparer = [](const std::pair<float, float>& a,
                                  const std::pair<float, float>& b) {
    return a.first < b.first;
  };

  Box3                b(pt, {pt.x, pt.y, DBL_MAX});
  std::vector<size_t> faces;
  faces.reserve(10);
  mFaceTree.queryBoxIntersects(b, std::back_inserter(faces));
  glm::vec3 triangles[3];
  glm::vec2 p2(pt);

  std::vector<std::pair<float, float>> hits;
  hits.reserve(faces.size());

  for (size_t fi : faces) {
    Face      f       = mFaces[fi];
    glm::vec3 pts3[3] = {mVertices[f.a], mVertices[f.b], mVertices[f.c]};
    glm::vec2 pts2[3] = {{pts3[0]}, {pts3[1]}, {pts3[2]}};
    float     bary[3];
    utils::barycentricCoords(pts2, p2, bary);
    if (!utils::barycentricWithinBounds(bary))
      continue;
    hits.emplace_back(faceNormal(fi).z, utils::barycentricEvaluate(bary, pts3).z);
  }

  std::sort(hits.begin(), hits.end(), comparer);
  bool   first = true;
  float  last  = 0;
  size_t count = 0;
  for (const std::pair<float, float>& hit : hits) {
    if (first || last * hit.second < 0)
      count++;
    last  = hit.second;
    first = false;
  }

  return count % 2;
}

void Mesh::clipWithPlane(const Plane& plane)
{
  const glm::vec3& pt     = plane.origin();
  const glm::vec3& normal = plane.normal();
  glm::vec3        unorm  = glm::normalize(normal);

  // Calculate vertex distances.
  std::vector<float> vdistances(numVertices());
  std::transform(
    vertexCBegin(), vertexCEnd(), vdistances.data(), [&pt, &unorm](const glm::vec3& v) {
      return glm::dot(v - pt, unorm);
    });

  // Compute edge-plane intersection points.
  std::vector<glm::vec3> edgepts(mEdges.size());
  for (size_t ei = 0; ei < mEdges.size(); ei++) {
    const EdgeType& edge = mEdges.at(ei);
    float           d1   = vdistances[edge.p];
    float           d2   = vdistances[edge.q];
    if (d1 * d2 >= 0)
      continue;

    float r     = d2 / (d2 - d1);
    edgepts[ei] = (vertex(edge.p) * r) + (vertex(edge.q) * (1.0f - r));
  }

  // Compute vertex enums for all faces.
  std::vector<uint8_t> venums(numFaces());
  std::transform(
    faceCBegin(), faceCEnd(), venums.data(), [&vdistances](const Face& face) {
      uint8_t mask = 0u;
      if (vdistances[face.a] < 0)
        mask |= 1 << 0;
      if (vdistances[face.b] < 0)
        mask |= 1 << 1;
      if (vdistances[face.c] < 0)
        mask |= 1 << 2;
      return mask;
    });

  // Total number of triangle indices.
  std::vector<Face> faces;
  size_t            nIndices = 0;
  for (const uint8_t venum : venums)
    nIndices += s_clipVertCountTable[venum];
  assert(nIndices % 3 == 0);

  size_t nFaces = nIndices / 3;

  // Copy the indices and vertices.
  std::unordered_map<size_t, size_t, CustomSizeTHash> map;
  map.reserve(nIndices);
  std::vector<glm::vec3> verts;
  verts.reserve(numVertices());

  faces.reserve(nFaces);

  std::vector<size_t> tempIndices;
  tempIndices.reserve(6);
  for (size_t fi = 0; fi < mFaces.size(); fi++) {
    const Face&          face  = mFaces.at(fi);
    uint8_t              venum = venums[fi];
    const uint8_t* const row   = s_clipTriTable[venum].data();
    tempIndices.clear();
    std::transform(row,
                   row + s_clipVertCountTable[venum],
                   std::back_inserter(tempIndices),
                   [this, &map, &verts, &edgepts, &face](const uint8_t vi) {
                     decltype(mEdgeIndexMap)::const_iterator match2;
                     if (vi > 2) {
                       match2 = mEdgeIndexMap.find(face.edge(vi - 3));
                       if (match2 == mEdgeIndexMap.end())
                         throw 1;
                     }
                     size_t key;
                     switch (vi) {
                     case 0:
                       key = face.a;
                       break;
                     case 1:
                       key = face.b;
                       break;
                     case 2:
                       key = face.c;
                       break;
                     case 3:
                     case 4:
                     case 5:
                       key = match2->second + numVertices();
                       break;
                     }
                     auto match = map.find(key);
                     if (match == map.end()) {
                       size_t vi2 = verts.size();
                       map.emplace(key, vi2);

                       switch (vi) {
                       case 0:
                         verts.push_back(vertex(face.a));
                         break;
                       case 1:
                         verts.push_back(vertex(face.b));
                         break;
                       case 2:
                         verts.push_back(vertex(face.c));
                         break;
                       case 3:
                       case 4:
                       case 5:
                         verts.push_back(edgepts[match2->second]);
                         break;
                       }
                       return vi2;
                     }
                     return match->second;
                   });

    for (size_t fvi = 0; fvi < tempIndices.size(); fvi += 3) {
      faces.emplace_back(tempIndices.data() + fvi);
    }
  }

  assert(faces.size() * 3 == nIndices);
  // Create new mesh with the copied data.
  mVertices = std::move(verts);
  mFaces    = std::move(faces);
  computeCache();
}

void Mesh::transform(const glm::mat4& mat)
{
  for (auto& v : mVertices) {
    v = glm::vec3(mat * glm::vec4(v.x, v.y, v.z, 1.0f));
  }
  computeRTrees();
  computeNormals();
}

glm::vec3 Mesh::closestPoint(const glm::vec3& pt, float searchDist) const
{
  size_t nearestVertIndex = SIZE_MAX;
  mVertexTree.queryNearestN(pt, 1, &nearestVertIndex);
  if (nearestVertIndex == SIZE_MAX)  // Didn't find the nearest vertex.
    return vec3_unset;

  glm::vec3 closePt    = mVertices[nearestVertIndex];
  float     bestDistSq = glm::length2(pt - closePt);

  float vDist = std::sqrt(bestDistSq);
  if (vDist > searchDist)  // Closest point not found within search distance.
    return vec3_unset;

  glm::vec3           halfDiag(vDist, vDist, vDist);
  std::vector<size_t> candidates;
  candidates.reserve(32);
  mFaceTree.queryBoxIntersects(Box3(pt - halfDiag, pt + halfDiag),
                               std::back_inserter(candidates));

  for (size_t fi : candidates) {
    faceClosestPt(fi, pt, closePt, bestDistSq);
  }
  return closePt;
}

Mesh::EdgeTriplet::EdgeTriplet(size_t const (&indices)[3])
    : a(indices[0])
    , b(indices[1])
    , c(indices[2])
{}

Mesh::EdgeTriplet::EdgeTriplet(size_t p, size_t q, size_t r)
    : a(p)
    , b(q)
    , c(r)
{}

void Mesh::EdgeTriplet::set(size_t p, size_t q, size_t r)
{
  a = p;
  b = q;
  c = r;
}

void Mesh::EdgeTriplet::set(size_t i)
{
  if (a == -1)
    a = i;
  else if (b == -1)
    b = i;
  else if (c == -1)
    c = i;
  else
    throw i;
}

Mesh Mesh::extractFaces(const std::vector<size_t>& faceIndices)
{
  std::vector<glm::vec3> vertices;
  std::vector<Face>      faces;
  faces.reserve(faceIndices.size());
  vertices.reserve(faceIndices.size() / 2);

  std::unordered_map<size_t, size_t, CustomSizeTHash> map;

  for (const auto& fi : faceIndices) {
    const Face& f = mFaces.at(fi);
    Face        face;
    for (uint8_t i = 0; i < 3; i++) {
      size_t fvi   = f.indices[i];
      auto   match = map.find(fvi);
      if (match == map.end()) {
        map.insert(std::make_pair(fvi, vertices.size()));
        face.indices[i] = vertices.size();
        vertices.push_back(mVertices[fvi]);
      }
      else {
        face.indices[i] = match->second;
      }
    }
    faces.push_back(face);
  }

  return Mesh(std::move(vertices), std::move(faces));
};

}  // namespace gal
