#include "galcore/Mesh.h"
#define _USE_MATH_DEFINES
#include <array>
#include <math.h>
#include <numeric>

static constexpr uint8_t X = UINT8_MAX;
static constexpr std::array<std::array<uint8_t, 6>, 8> s_clipTriTable{{
    {X, X, X, X, X, X},
    {0, 3, 5, X, X, X},
    {3, 1, 4, X, X, X},
    {0, 1, 5, 1, 4, 5},
    {4, 2, 5, X, X, X},
    {0, 3, 4, 0, 4, 2},
    {1, 5, 3, 1, 2, 5},
    {0, 1, 2, X, X, X},
}};

static constexpr std::array<uint8_t, 8> s_clipVertCountTable{0, 3, 3, 6,
                                                             3, 6, 6, 3};

const Mesh::Face Mesh::Face::unset = Face(-1, -1, -1);

Mesh::Face::Face() : a(SIZE_MAX), b(SIZE_MAX), c(SIZE_MAX) {}

Mesh::Face::Face(size_t v1, size_t v2, size_t v3) : a(v1), b(v2), c(v3) {}

Mesh::Face::Face(size_t const indices[3])
    : Face(indices[0], indices[1], indices[2]) {}

void Mesh::Face::flip() {
  size_t temp = c;
  c = b;
  b = temp;
}

EdgeType Mesh::Face::edge(uint8_t edgeIndex) const {
  switch (edgeIndex) {
  case 0:
    return IndexPair(a, b);
  case 1:
    return IndexPair(b, c);
  case 2:
    return IndexPair(c, a);
  default:
    throw edgeIndex;
  }
}

bool Mesh::Face::containsVertex(size_t vertIndex) const {
  return a == vertIndex || b == vertIndex || c == vertIndex;
}

bool Mesh::Face::isDegenerate() const { return a == b || b == c || c == a; }

void Mesh::computeCache() {
  computeRTrees();
  computeTopology();
  computeNormals();
  checkSolid();
}

void Mesh::computeTopology() {
  size_t nVertices = numVertices();
  size_t curEi = 0;
  for (size_t fi = 0; fi < mFaces.size(); fi++) {
    Face f = mFaces[fi];
    mVertFaces[f.a].push_back(fi);
    mVertFaces[f.b].push_back(fi);
    mVertFaces[f.c].push_back(fi);

    addEdges(f, fi);
  }
}

void Mesh::computeRTrees() {
  for (size_t fi = 0; fi < mFaces.size(); fi++) {
    mFaceTree.insert(faceBounds(fi), fi);
  }

  size_t vi = 0;
  for (const vec3 &v : mVertices) {
    mVertexTree.insert(box3(v), vi++);
  }
}

void Mesh::computeNormals() {
  mFaceNormals.clear();
  mFaceNormals.reserve(mFaces.size());
  for (ConstFaceIter fi = faceCBegin(); fi != faceCEnd(); fi++) {
    Face f = *fi;
    vec3 a = mVertices[f.a];
    vec3 b = mVertices[f.b];
    vec3 c = mVertices[f.c];
    mFaceNormals.push_back(((b - a) ^ (c - a)).unit());
  }

  mVertexNormals.clear();
  mVertexNormals.resize(mVertices.size());
  std::vector<vec3> faceNormals;
  for (size_t vi = 0; vi < mVertices.size(); vi++) {
    const auto &faces = mVertFaces.at(vi);
    faceNormals.clear();
    faceNormals.reserve(faces.size());
    std::transform(faces.cbegin(), faces.cend(),
                   std::back_inserter(faceNormals),
                   [this](const size_t fi) { return mFaceNormals[fi]; });
    mVertexNormals[vi].set(
        vec3::average(faceNormals.cbegin(), faceNormals.cend()).unit());
  }
}

void Mesh::addEdge(const Face &f, size_t fi, uint8_t fei, size_t &newEi) {
  EdgeType e = f.edge(fei);
  auto eMatch = mEdgeIndexMap.find(e);
  if (eMatch == mEdgeIndexMap.end()) {
    newEi = mEdges.size();
    mEdgeIndexMap.emplace(e, newEi);
    mEdges.push_back(e);
    mEdgeFaces.emplace_back();

    mVertEdges[e.p].push_back(newEi);
    mVertEdges[e.q].push_back(newEi);
  } else {
    newEi = eMatch->second;
  }

  mEdgeFaces[newEi].push_back(fi);
}

void Mesh::addEdges(const Face &f, size_t fi) {
  size_t indices[3];
  for (uint8_t fei = 0; fei < 3; fei++) {
    addEdge(f, fi, fei, indices[fei]);
  }
  mFaceEdges[fi] = EdgeTriplet(indices);
}

double Mesh::faceArea(const Face &f) const {
  vec3 a = vertex(f.a);
  return (vertex(f.b) - a ^ vertex(f.c) - a).len() * 0.5;
}

void Mesh::getFaceCenter(const Face &f, vec3 &center) const {
  center = (mVertices[f.a] + mVertices[f.b] + mVertices[f.c]) / 3.0;
}

void Mesh::checkSolid() {
  for (const auto &faces : mEdgeFaces) {
    if (faces.size() != 2) {
      mIsSolid = false;
      return;
    }
  }
  mIsSolid = true;
}

vec3 Mesh::areaCentroid() const {
  std::vector<vec3> centers;
  centers.reserve(numFaces());
  std::vector<double> areas;
  areas.reserve(numFaces());

  for (ConstFaceIter fIter = faceCBegin(); fIter != faceCEnd();
       fIter++) {
    vec3 center;
    Face f = *fIter;
    getFaceCenter(f, center);
    centers.push_back(center);
    areas.push_back(faceArea(f));
  }

  return vec3::weighted_average(centers.cbegin(), centers.cend(),
                                areas.cbegin(), areas.cend());
}

vec3 Mesh::volumeCentroid() const {
  vec3 refPt = bounds().center();
  std::vector<vec3> joinVecs;
  joinVecs.reserve(numVertices());
  std::transform(vertexCBegin(), vertexCEnd(), std::back_inserter(joinVecs),
                 [&refPt](const vec3 &vert) { return vert - refPt; });

  std::vector<vec3> centers;
  centers.reserve(numFaces());
  std::vector<double> volumes;
  volumes.reserve(numFaces());

  for (size_t fi = 0; fi < mFaces.size(); fi++) {
    Face f = mFaces[fi];
    vec3 a = joinVecs[f.a];
    vec3 b = joinVecs[f.b];
    vec3 c = joinVecs[f.c];

    double volume = std::abs((a ^ b) * c) / 6.0;
    if (a * faceNormal(fi) < 0) {
      volume *= -1;
    }
    volumes.push_back(volume);

    a = mVertices[f.a];
    b = mVertices[f.b];
    c = mVertices[f.c];
    centers.push_back((a + b + c + refPt) * 0.25);
  }

  return vec3::weighted_average(centers.cbegin(), centers.cend(),
                                volumes.cbegin(), volumes.cend());
}

const RTree3d &Mesh::elementTree(eMeshElement element) const {
  switch (element) {
  case eMeshElement::face:
    return mFaceTree;
  case eMeshElement::vertex:
    return mVertexTree;
  default:
    throw "Invalid element type";
  }
}

void Mesh::faceClosestPt(size_t faceIndex, const vec3 &pt, vec3 &closePt,
                         double &bestSqDist) const {
  const Face &face = mFaces.at(faceIndex);
  const vec3 &va = mVertices.at(face.a);
  const vec3 &fnorm = faceNormal(faceIndex);
  vec3 projection = fnorm * ((va - pt) * fnorm);

  double planeDistSq = projection.len_sq();
  if (planeDistSq > bestSqDist)
    return;

  vec3 projected = pt + projection;

  uint8_t nOutside = 0;
  for (uint8_t i = 0; i < 3; i++) {
    const vec3 &v1 = mVertices.at(face.indices[i]);
    const vec3 &v2 = mVertices.at(face.indices[(i + 1) % 3]);
    bool outside = ((v1 - projected) ^ (v2 - projected)) * fnorm < 0.0;
    if (outside) {
      nOutside++;
      vec3 ln = v2 - v1;
      double param =
          std::clamp((ln * (projected - v1)) / ln.len_sq(), 0.0, 1.0);
      vec3 cpt = v2 * param + v1 * (1.0 - param);
      double distSq = (cpt - pt).len_sq();
      if (distSq < bestSqDist) {
        closePt = cpt;
        bestSqDist = distSq;
      }
    }

    if (nOutside > 1)
      break;
  }

  if (nOutside == 0) {
    closePt = projected;
    bestSqDist = planeDistSq;
  }
}

Mesh::Mesh(const Mesh &other)
    : Mesh(other.mVertices.data(), other.numVertices(), other.mFaces.data(),
           other.numFaces()) {}

Mesh::Mesh(const vec3 *verts, size_t nVerts, const Face *faces,
           size_t nFaces)
    : mVertEdges(nVerts), mVertFaces(nVerts), mFaceEdges(nFaces) {
  mVertices.reserve(nVerts);
  std::copy(verts, verts + nVerts, std::back_inserter(mVertices));
  mFaces.reserve(nFaces);
  std::copy(faces, faces + nFaces, std::back_inserter(mFaces));
  computeCache();
}

Mesh::Mesh(const double *vertCoords, size_t nVerts,
           const size_t *faceVertIndices, size_t nFaces)
    : mVertEdges(nVerts), mVertFaces(nVerts), mFaceEdges(nFaces) {
  mVertices.reserve(nVerts);
  size_t nFlat = nVerts * 3;
  size_t i = 0;
  while (i < nFlat) {
    double x = vertCoords[i++];
    double y = vertCoords[i++];
    double z = vertCoords[i++];
    mVertices.emplace_back(x, y, z);
  }

  mFaces.reserve(nFaces);
  nFlat = nFaces * 3;
  i = 0;
  while (i < nFlat) {
    size_t a = faceVertIndices[i++];
    size_t b = faceVertIndices[i++];
    size_t c = faceVertIndices[i++];
    mFaces.emplace_back(a, b, c);
  }

  computeCache();
}

size_t Mesh::numVertices() const noexcept { return mVertices.size(); }

size_t Mesh::numFaces() const noexcept { return mFaces.size(); }

vec3 Mesh::vertex(size_t vi) const {
  return vi < numVertices() ? mVertices[vi] : vec3::unset;
}

Mesh::Face Mesh::face(size_t fi) const {
  return fi < numFaces() ? mFaces[fi] : Face::unset;
}

vec3 Mesh::vertexNormal(size_t vi) const {
  return vi < numVertices() ? mVertexNormals[vi] : vec3::unset;
}

const vec3 &Mesh::faceNormal(size_t fi) const {
  static const vec3 s_unset = vec3::unset;
  return fi < numFaces() ? mFaceNormals.at(fi) : s_unset;
}

Mesh::ConstVertIter Mesh::vertexCBegin() const {
  return mVertices.cbegin();
}

Mesh::ConstVertIter Mesh::vertexCEnd() const {
  return mVertices.cend();
}

Mesh::ConstFaceIter Mesh::faceCBegin() const { return mFaces.cbegin(); }

Mesh::ConstFaceIter Mesh::faceCEnd() const { return mFaces.cend(); }

box3 Mesh::bounds() const {
  box3 b;
  for (const vec3 &v : mVertices) {
    b.inflate(v);
  }
  return b;
}

double Mesh::faceArea(size_t fi) const { return faceArea(face(fi)); }

double Mesh::area() const {
  double sum = 0;
  for (const Face &f : mFaces) {
    sum += faceArea(f);
  }
  return sum;
}

box3 Mesh::faceBounds(size_t fi) const {
  box3 b;
  Face f = mFaces[fi];
  b.inflate(mVertices[f.a]);
  b.inflate(mVertices[f.b]);
  b.inflate(mVertices[f.c]);
  return b;
}

double Mesh::volume() const {
  if (!isSolid())
    return 0.0;

  vec3 refPt = bounds().center();
  std::vector<vec3> joinVectors;
  joinVectors.reserve(numVertices());
  std::transform(vertexCBegin(), vertexCEnd(),
                 std::back_inserter(joinVectors),
                 [&refPt](const vec3 vert) { return vert - refPt; });

  double total = 0.0;
  for (size_t fi = 0; fi < mFaces.size(); fi++) {
    Face face = mFaces[fi];
    vec3 a = joinVectors[face.a];
    vec3 b = joinVectors[face.b];
    vec3 c = joinVectors[face.c];
    double tetVolume = std::abs((a ^ b) * c) / 6.0;

    if (a * faceNormal(fi) > 0)
      total += tetVolume;
    else
      total -= tetVolume;
  }

  return total;
}

bool Mesh::isSolid() const { return mIsSolid; }

vec3 Mesh::centroid() const { return centroid(eMeshCentroidType::vertexBased); }

vec3 Mesh::centroid(const eMeshCentroidType centroid_type) const {
  switch (centroid_type) {
  case eMeshCentroidType::vertexBased:
    return vec3::average(vertexCBegin(), vertexCEnd());
  case eMeshCentroidType::areaBased:
    return areaCentroid();
  case eMeshCentroidType::volumeBased:
    return volumeCentroid();
  default:
    return centroid(eMeshCentroidType::vertexBased);
  }
}

bool Mesh::contains(const vec3 &pt) const {
  static const auto comparer = [](const std::pair<double, double> &a,
                                  const std::pair<double, double> &b) {
    return a.first < b.first;
  };

  box3 b(pt, {pt.x, pt.y, DBL_MAX});
  std::vector<size_t> faces;
  faces.reserve(10);
  mFaceTree.queryBoxIntersects(b, std::back_inserter(faces));
  vec3 triangles[3];
  vec2 p2(pt);

  std::vector<std::pair<double, double>> hits;
  hits.reserve(faces.size());

  for (size_t fi : faces) {
    Face f = mFaces[fi];
    vec3 pts3[3] = {mVertices[f.a], mVertices[f.b], mVertices[f.c]};
    vec2 pts2[3] = {{pts3[0]}, {pts3[1]}, {pts3[2]}};
    double bary[3];
    utils::barycentricCoords(pts2, p2, bary);
    if (!utils::barycentricWithinBounds(bary))
      continue;
    hits.emplace_back(faceNormal(fi).z,
                      utils::barycentricEvaluate(bary, pts3).z);
  }

  std::sort(hits.begin(), hits.end(), comparer);
  bool first = true;
  double last = 0;
  size_t count = 0;
  for (const std::pair<double, double> &hit : hits) {
    if (first || last * hit.second < 0)
      count++;
    last = hit.second;
    first = false;
  }

  return count % 2;
}

Mesh *Mesh::clippedWithPlane(const vec3 &pt, const vec3 &normal) const {
  vec3 unorm = normal.unit();

  // Calculate vertex distances.
  std::vector<double> vdistances(numVertices());
  std::transform(vertexCBegin(), vertexCEnd(), vdistances.data(),
                 [&pt, &unorm](const vec3 &v) { return (v - pt) * unorm; });

  // Compute edge-plane intersection points.
  std::vector<vec3> edgepts(mEdges.size());
  for (size_t ei = 0; ei < mEdges.size(); ei++) {
    const EdgeType &edge = mEdges.at(ei);
    double d1 = vdistances[edge.p];
    double d2 = vdistances[edge.q];
    if (d1 * d2 >= 0)
      continue;

    double r = d2 / (d2 - d1);
    edgepts[ei] = (vertex(edge.p) * r) + (vertex(edge.q) * (1.0 - r));
  }

  // Compute vertex enums for all faces.
  std::vector<uint8_t> venums(numFaces());
  std::transform(faceCBegin(), faceCEnd(), venums.data(),
                 [&vdistances](const Face &face) {
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
  std::vector<size_t> indices;
  size_t nIndices = 0;
  for (const uint8_t venum : venums)
    nIndices += s_clipVertCountTable[venum];
  assert(nIndices % 3 == 0);

  // Copy the indices and vertices.
  std::unordered_map<size_t, size_t, CustomSizeTHash> map;
  map.reserve(nIndices);
  std::vector<vec3> verts;
  verts.reserve(numVertices());

  indices.reserve(nIndices);
  auto indexIt = std::back_inserter(indices);

  for (size_t fi = 0; fi < mFaces.size(); fi++) {
    const Face &face = mFaces.at(fi);
    uint8_t venum = venums[fi];
    const uint8_t *const row = s_clipTriTable[venum].data();
    std::transform(row, row + s_clipVertCountTable[venum], indexIt,
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
  }

  assert(indices.size() == nIndices);
  // Create new mesh with the copied data.
  return new Mesh((const double *)verts.data(), verts.size(), indices.data(),
                  nIndices / 3);
}

vec3 Mesh::closestPoint(const vec3 &pt, double searchDist) const {
  size_t nearestVertIndex = SIZE_MAX;
  mVertexTree.queryNearestN(pt, 1, &nearestVertIndex);
  if (nearestVertIndex == SIZE_MAX) // Didn't find the nearest vertex.
    return vec3::unset;

  vec3 closePt = mVertices[nearestVertIndex];
  double bestDistSq = (pt - closePt).len_sq();

  double vDist = std::sqrt(bestDistSq);
  if (vDist > searchDist) // Closest point not found within search distance.
    return vec3::unset;

  vec3 halfDiag(vDist, vDist, vDist);
  std::vector<size_t> candidates;
  candidates.reserve(32);
  mFaceTree.queryBoxIntersects(box3(pt - halfDiag, pt + halfDiag),
                                  std::back_inserter(candidates));

  for (size_t fi : candidates) {
    faceClosestPt(fi, pt, closePt, bestDistSq);
  }
  return closePt;
}

Mesh::EdgeTriplet::EdgeTriplet(size_t const (&indices)[3])
    : a(indices[0]), b(indices[1]), c(indices[2]) {}

Mesh::EdgeTriplet::EdgeTriplet(size_t p, size_t q, size_t r) : a(p), b(q), c(r) {}

void Mesh::EdgeTriplet::set(size_t p, size_t q, size_t r) {
  a = p;
  b = q;
  c = r;
}

void Mesh::EdgeTriplet::set(size_t i) {
  if (a == -1)
    a = i;
  else if (b == -1)
    b = i;
  else if (c == -1)
    c = i;
  else
    throw i;
}
