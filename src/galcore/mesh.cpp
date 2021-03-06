#include "galcore/mesh.h"
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

const meshFace meshFace::unset = meshFace(-1, -1, -1);

meshFace::meshFace() : a(SIZE_MAX), b(SIZE_MAX), c(SIZE_MAX) {}

meshFace::meshFace(size_t v1, size_t v2, size_t v3) : a(v1), b(v2), c(v3) {}

meshFace::meshFace(size_t const indices[3])
    : meshFace(indices[0], indices[1], indices[2]) {}

void meshFace::flip() {
  size_t temp = c;
  c = b;
  b = temp;
}

edge_type meshFace::edge(uint8_t edgeIndex) const {
  switch (edgeIndex) {
  case 0:
    return indexPair(a, b);
  case 1:
    return indexPair(b, c);
  case 2:
    return indexPair(c, a);
  default:
    throw edgeIndex;
  }
}

bool meshFace::containsVertex(size_t vertIndex) const {
  return a == vertIndex || b == vertIndex || c == vertIndex;
}

bool meshFace::is_degenerate() const { return a == b || b == c || c == a; }

void mesh::computeCache() {
  computeRTrees();
  computeTopology();
  computeNormals();
  checkSolid();
}

void mesh::computeTopology() {
  size_t nVertices = num_vertices();
  size_t curEi = 0;
  for (size_t fi = 0; fi < mFaces.size(); fi++) {
    meshFace f = mFaces[fi];
    mVertFaces[f.a].push_back(fi);
    mVertFaces[f.b].push_back(fi);
    mVertFaces[f.c].push_back(fi);

    addEdges(f, fi);
  }
}

void mesh::computeRTrees() {
  for (size_t fi = 0; fi < mFaces.size(); fi++) {
    m_faceTree.insert(face_bounds(fi), fi);
  }

  size_t vi = 0;
  for (const vec3 &v : mVertices) {
    mVertexTree.insert(box3(v), vi++);
  }
}

void mesh::computeNormals() {
  mFaceNormals.clear();
  mFaceNormals.reserve(mFaces.size());
  for (const_face_iterator fi = face_cbegin(); fi != face_cend(); fi++) {
    meshFace f = *fi;
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

void mesh::addEdge(const meshFace &f, size_t fi, uint8_t fei, size_t &newEi) {
  edge_type e = f.edge(fei);
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

void mesh::addEdges(const meshFace &f, size_t fi) {
  size_t indices[3];
  for (uint8_t fei = 0; fei < 3; fei++) {
    addEdge(f, fi, fei, indices[fei]);
  }
  mFaceEdges[fi] = faceEdges(indices);
}

double mesh::faceArea(const meshFace &f) const {
  vec3 a = vertex(f.a);
  return (vertex(f.b) - a ^ vertex(f.c) - a).len() * 0.5;
}

void mesh::getFaceCenter(const meshFace &f, vec3 &center) const {
  center = (mVertices[f.a] + mVertices[f.b] + mVertices[f.c]) / 3.0;
}

void mesh::checkSolid() {
  for (const auto &faces : mEdgeFaces) {
    if (faces.size() != 2) {
      mIsSolid = false;
      return;
    }
  }
  mIsSolid = true;
}

vec3 mesh::areaCentroid() const {
  std::vector<vec3> centers;
  centers.reserve(numFaces());
  std::vector<double> areas;
  areas.reserve(numFaces());

  for (const_face_iterator fIter = face_cbegin(); fIter != face_cend();
       fIter++) {
    vec3 center;
    meshFace f = *fIter;
    getFaceCenter(f, center);
    centers.push_back(center);
    areas.push_back(faceArea(f));
  }

  return vec3::weighted_average(centers.cbegin(), centers.cend(),
                                areas.cbegin(), areas.cend());
}

vec3 mesh::volumeCentroid() const {
  vec3 refPt = bounds().center();
  std::vector<vec3> joinVecs;
  joinVecs.reserve(num_vertices());
  std::transform(vertex_cbegin(), vertex_cend(), std::back_inserter(joinVecs),
                 [&refPt](const vec3 &vert) { return vert - refPt; });

  std::vector<vec3> centers;
  centers.reserve(numFaces());
  std::vector<double> volumes;
  volumes.reserve(numFaces());

  for (size_t fi = 0; fi < mFaces.size(); fi++) {
    meshFace f = mFaces[fi];
    vec3 a = joinVecs[f.a];
    vec3 b = joinVecs[f.b];
    vec3 c = joinVecs[f.c];

    double volume = std::abs((a ^ b) * c) / 6.0;
    if (a * face_normal(fi) < 0) {
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

const rtree3d &mesh::element_tree(meshElement element) const {
  switch (element) {
  case meshElement::face:
    return m_faceTree;
  case meshElement::vertex:
    return mVertexTree;
  default:
    throw "Invalid element type";
  }
}

void mesh::faceClosestPt(size_t faceIndex, const vec3 &pt, vec3 &closePt,
                         double &bestSqDist) const {
  const meshFace &face = mFaces.at(faceIndex);
  const vec3 &va = mVertices.at(face.a);
  const vec3 &fnorm = face_normal(faceIndex);
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

mesh::mesh(const mesh &other)
    : mesh(other.mVertices.data(), other.num_vertices(), other.mFaces.data(),
           other.numFaces()) {}

mesh::mesh(const vec3 *verts, size_t nVerts, const meshFace *faces,
           size_t nFaces)
    : mVertEdges(nVerts), mVertFaces(nVerts), mFaceEdges(nFaces) {
  mVertices.reserve(nVerts);
  std::copy(verts, verts + nVerts, std::back_inserter(mVertices));
  mFaces.reserve(nFaces);
  std::copy(faces, faces + nFaces, std::back_inserter(mFaces));
  computeCache();
}

mesh::mesh(const double *vertCoords, size_t nVerts,
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

size_t mesh::num_vertices() const noexcept { return mVertices.size(); }

size_t mesh::numFaces() const noexcept { return mFaces.size(); }

vec3 mesh::vertex(size_t vi) const {
  return vi < num_vertices() ? mVertices[vi] : vec3::unset;
}

meshFace mesh::face(size_t fi) const {
  return fi < numFaces() ? mFaces[fi] : meshFace::unset;
}

vec3 mesh::vertex_normal(size_t vi) const {
  return vi < num_vertices() ? mVertexNormals[vi] : vec3::unset;
}

const vec3 &mesh::face_normal(size_t fi) const {
  static const vec3 s_unset = vec3::unset;
  return fi < numFaces() ? mFaceNormals.at(fi) : s_unset;
}

mesh::const_vertex_iterator mesh::vertex_cbegin() const {
  return mVertices.cbegin();
}

mesh::const_vertex_iterator mesh::vertex_cend() const {
  return mVertices.cend();
}

mesh::const_face_iterator mesh::face_cbegin() const { return mFaces.cbegin(); }

mesh::const_face_iterator mesh::face_cend() const { return mFaces.cend(); }

box3 mesh::bounds() const {
  box3 b;
  for (const vec3 &v : mVertices) {
    b.inflate(v);
  }
  return b;
}

double mesh::faceArea(size_t fi) const { return faceArea(face(fi)); }

double mesh::area() const {
  double sum = 0;
  for (const meshFace &f : mFaces) {
    sum += faceArea(f);
  }
  return sum;
}

box3 mesh::face_bounds(size_t fi) const {
  box3 b;
  meshFace f = mFaces[fi];
  b.inflate(mVertices[f.a]);
  b.inflate(mVertices[f.b]);
  b.inflate(mVertices[f.c]);
  return b;
}

double mesh::volume() const {
  if (!is_solid())
    return 0.0;

  vec3 refPt = bounds().center();
  std::vector<vec3> joinVectors;
  joinVectors.reserve(num_vertices());
  std::transform(vertex_cbegin(), vertex_cend(),
                 std::back_inserter(joinVectors),
                 [&refPt](const vec3 vert) { return vert - refPt; });

  double total = 0.0;
  for (size_t fi = 0; fi < mFaces.size(); fi++) {
    meshFace face = mFaces[fi];
    vec3 a = joinVectors[face.a];
    vec3 b = joinVectors[face.b];
    vec3 c = joinVectors[face.c];
    double tetVolume = std::abs((a ^ b) * c) / 6.0;

    if (a * face_normal(fi) > 0)
      total += tetVolume;
    else
      total -= tetVolume;
  }

  return total;
}

bool mesh::is_solid() const { return mIsSolid; }

vec3 mesh::centroid() const { return centroid(meshCentroidType::vertex_based); }

vec3 mesh::centroid(const meshCentroidType centroid_type) const {
  switch (centroid_type) {
  case meshCentroidType::vertex_based:
    return vec3::average(vertex_cbegin(), vertex_cend());
  case meshCentroidType::area_based:
    return areaCentroid();
  case meshCentroidType::volume_based:
    return volumeCentroid();
  default:
    return centroid(meshCentroidType::vertex_based);
  }
}

bool mesh::contains(const vec3 &pt) const {
  static const auto comparer = [](const std::pair<double, double> &a,
                                  const std::pair<double, double> &b) {
    return a.first < b.first;
  };

  box3 b(pt, {pt.x, pt.y, DBL_MAX});
  std::vector<size_t> faces;
  faces.reserve(10);
  m_faceTree.query_box_intersects(b, std::back_inserter(faces));
  vec3 triangles[3];
  vec2 p2(pt);

  std::vector<std::pair<double, double>> hits;
  hits.reserve(faces.size());

  for (size_t fi : faces) {
    meshFace f = mFaces[fi];
    vec3 pts3[3] = {mVertices[f.a], mVertices[f.b], mVertices[f.c]};
    vec2 pts2[3] = {{pts3[0]}, {pts3[1]}, {pts3[2]}};
    double bary[3];
    utils::barycentricCoords(pts2, p2, bary);
    if (!utils::barycentricWithinBounds(bary))
      continue;
    hits.emplace_back(face_normal(fi).z,
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

mesh *mesh::clipped_with_plane(const vec3 &pt, const vec3 &normal) const {
  vec3 unorm = normal.unit();

  // Calculate vertex distances.
  std::vector<double> vdistances(num_vertices());
  std::transform(vertex_cbegin(), vertex_cend(), vdistances.data(),
                 [&pt, &unorm](const vec3 &v) { return (v - pt) * unorm; });

  // Compute edge-plane intersection points.
  std::vector<vec3> edgepts(mEdges.size());
  for (size_t ei = 0; ei < mEdges.size(); ei++) {
    const edge_type &edge = mEdges.at(ei);
    double d1 = vdistances[edge.p];
    double d2 = vdistances[edge.q];
    if (d1 * d2 >= 0)
      continue;

    double r = d2 / (d2 - d1);
    edgepts[ei] = (vertex(edge.p) * r) + (vertex(edge.q) * (1.0 - r));
  }

  // Compute vertex enums for all faces.
  std::vector<uint8_t> venums(numFaces());
  std::transform(face_cbegin(), face_cend(), venums.data(),
                 [&vdistances](const meshFace &face) {
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
  std::unordered_map<size_t, size_t, customSizeTHash> map;
  map.reserve(nIndices);
  std::vector<vec3> verts;
  verts.reserve(num_vertices());

  indices.reserve(nIndices);
  auto indexIt = std::back_inserter(indices);

  for (size_t fi = 0; fi < mFaces.size(); fi++) {
    const meshFace &face = mFaces.at(fi);
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
                       key = match2->second + num_vertices();
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
  return new mesh((const double *)verts.data(), verts.size(), indices.data(),
                  nIndices / 3);
}

vec3 mesh::closest_point(const vec3 &pt, double searchDist) const {
  size_t nearestVertIndex = SIZE_MAX;
  mVertexTree.query_nearest_n(pt, 1, &nearestVertIndex);
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
  m_faceTree.query_box_intersects(box3(pt - halfDiag, pt + halfDiag),
                                  std::back_inserter(candidates));

  for (size_t fi : candidates) {
    faceClosestPt(fi, pt, closePt, bestDistSq);
  }
  return closePt;
}

faceEdges::faceEdges(size_t const indices[3])
    : a(indices[0]), b(indices[1]), c(indices[2]) {}

faceEdges::faceEdges(size_t p, size_t q, size_t r) : a(p), b(q), c(r) {}

void faceEdges::set(size_t p, size_t q, size_t r) {
  a = p;
  b = q;
  c = r;
}

void faceEdges::set(size_t i) {
  if (a == -1)
    a = i;
  else if (b == -1)
    b = i;
  else if (c == -1)
    c = i;
  else
    throw i;
}
