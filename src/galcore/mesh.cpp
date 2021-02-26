#include "galcore/mesh.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <array>
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

static constexpr std::array<uint8_t, 8> s_clipVertCountTable{ 0, 3, 3, 6, 3, 6, 6, 3 };

const mesh_face mesh_face::unset = mesh_face(-1, -1, -1);

mesh_face::mesh_face() :
    a(SIZE_MAX), b(SIZE_MAX), c(SIZE_MAX)
{
}

mesh_face::mesh_face(size_t v1, size_t v2, size_t v3)
    :a(v1), b(v2), c(v3)
{
}

mesh_face::mesh_face(size_t const indices[3]) : mesh_face(indices[0], indices[1], indices[2])
{
}

void mesh_face::flip()
{
    size_t temp = c;
    c = b;
    b = temp;
}

edge_type mesh_face::edge(uint8_t edgeIndex) const
{
    switch (edgeIndex)
    {
    case 0:
        return index_pair(a, b);
    case 1:
        return index_pair(b, c);
    case 2:
        return index_pair(c, a);
    default:
        throw edgeIndex;
    }
}

bool mesh_face::contains_vertex(size_t vertIndex) const
{
    return a == vertIndex || b == vertIndex || c == vertIndex;
}

bool mesh_face::is_degenerate() const
{
    return a == b || b == c || c == a;
}

void mesh::compute_cache()
{
    compute_rtrees();
    compute_topology();
    compute_normals();
    check_solid();
}

void mesh::compute_topology()
{
    size_t nVertices = num_vertices();
    size_t curEi = 0;
    for (size_t fi = 0; fi < m_faces.size(); fi++)
    {
        mesh_face f = m_faces[fi];
        m_vertFaces[f.a].push_back(fi);
        m_vertFaces[f.b].push_back(fi);
        m_vertFaces[f.c].push_back(fi);

        add_edges(f, fi);
    }
}

void mesh::compute_rtrees()
{
    for (size_t fi = 0; fi < m_faces.size(); fi++)
    {
        m_faceTree.insert(face_bounds(fi), fi);
    }

    size_t vi = 0;
    for (const vec3& v : m_vertices)
    {
        m_vertexTree.insert(box3(v), vi++);
    }
}

void mesh::compute_normals()
{
    m_faceNormals.clear();
    m_faceNormals.reserve(m_faces.size());
    for (const_face_iterator fi = face_cbegin(); fi != face_cend(); fi++)
    {
        mesh_face f = *fi;
        vec3 a = m_vertices[f.a];
        vec3 b = m_vertices[f.b];
        vec3 c = m_vertices[f.c];
        m_faceNormals.push_back(((b - a) ^ (c - a)).unit());
    }

    m_vertexNormals.clear();
    m_vertexNormals.resize(m_vertices.size());
    std::vector<vec3> faceNormals;
    for (size_t vi = 0; vi < m_vertices.size(); vi++)
    {
        const auto& faces = m_vertFaces.at(vi);
        faceNormals.clear();
        faceNormals.reserve(faces.size());
        std::transform(faces.cbegin(), faces.cend(), std::back_inserter(faceNormals),
            [this](const size_t fi) { return m_faceNormals[fi]; });
        m_vertexNormals[vi].set(vec3::average(faceNormals.cbegin(), faceNormals.cend()).unit());
    }
}

void mesh::add_edge(const mesh_face& f, size_t fi, uint8_t fei, size_t& newEi)
{
    edge_type e = f.edge(fei);
    auto eMatch = m_edgeIndexMap.find(e);
    if (eMatch == m_edgeIndexMap.end())
    {
        newEi = m_edges.size();
        m_edgeIndexMap.emplace(e, newEi);
        m_edges.push_back(e);
        m_edgeFaces.emplace_back();

        m_vertEdges[e.p].push_back(newEi);
        m_vertEdges[e.q].push_back(newEi);
    }
    else
    {
        newEi = eMatch->second;
    }

    m_edgeFaces[newEi].push_back(fi);
}

void mesh::add_edges(const mesh_face& f, size_t fi)
{
    size_t indices[3];
    for (uint8_t fei = 0; fei < 3; fei++)
    {
        add_edge(f, fi, fei, indices[fei]);
    }
    m_faceEdges[fi] = face_edges(indices);
}

double mesh::face_area(const mesh_face& f) const
{
    vec3 a = vertex(f.a);
    return (vertex(f.b) - a ^ vertex(f.c) - a).len() * 0.5;
}

void mesh::get_face_center(const mesh_face& f, vec3& center) const
{
    center = (m_vertices[f.a] + m_vertices[f.b] + m_vertices[f.c]) / 3.0;
}

void mesh::check_solid()
{
    for (const auto& faces : m_edgeFaces)
    {
        if (faces.size() != 2)
        {
            m_isSolid = false;
            return;
        }
    }
    m_isSolid = true;
}

vec3 mesh::area_centroid() const
{
    std::vector<vec3> centers;
    centers.reserve(num_faces());
    std::vector<double> areas;
    areas.reserve(num_faces());

    for (const_face_iterator fIter = face_cbegin(); fIter != face_cend(); fIter++)
    {
        vec3 center;
        mesh_face f = *fIter;
        get_face_center(f, center);
        centers.push_back(center);
        areas.push_back(face_area(f));
    }

    return vec3::weighted_average(centers.cbegin(), centers.cend(), areas.cbegin(), areas.cend());
}

vec3 mesh::volume_centroid() const
{
    vec3 refPt = bounds().center();
    std::vector<vec3> joinVecs;
    joinVecs.reserve(num_vertices());
    std::transform(vertex_cbegin(), vertex_cend(), std::back_inserter(joinVecs),
        [&refPt](const vec3& vert) {
            return vert - refPt;
        });

    std::vector<vec3> centers;
    centers.reserve(num_faces());
    std::vector<double> volumes;
    volumes.reserve(num_faces());

    for (size_t fi = 0; fi < m_faces.size(); fi++)
    {
        mesh_face f = m_faces[fi];
        vec3 a = joinVecs[f.a];
        vec3 b = joinVecs[f.b];
        vec3 c = joinVecs[f.c];

        double volume = std::abs((a ^ b) * c) / 6.0;
        if (a * face_normal(fi) < 0)
        {
            volume *= -1;
        }
        volumes.push_back(volume);

        a = m_vertices[f.a];
        b = m_vertices[f.b];
        c = m_vertices[f.c];
        centers.push_back((a + b + c + refPt) * 0.25);
    }

    return vec3::weighted_average(centers.cbegin(), centers.cend(), volumes.cbegin(), volumes.cend());
}

const rtree3d& mesh::element_tree(mesh_element element) const
{
    switch (element)
    {
    case mesh_element::face: return m_faceTree;
    case mesh_element::vertex: return m_vertexTree;
    default: throw "Invalid element type";
    }
}

void mesh::face_closest_pt(size_t faceIndex, const vec3& pt, vec3& closePt, double& bestSqDist) const
{
    const mesh_face& face = m_faces.at(faceIndex);
    const vec3& va = m_vertices.at(face.a);
    const vec3& fnorm = face_normal(faceIndex);
    vec3 projection = fnorm * ((va - pt) * fnorm);

    double planeDistSq = projection.len_sq();
    if (planeDistSq > bestSqDist) return;

    vec3 projected = pt + projection;

    uint8_t nOutside = 0;
    for (uint8_t i = 0; i < 3; i++)
    {
        const vec3& v1 = m_vertices.at(face.indices[i]);
        const vec3& v2 = m_vertices.at(face.indices[(i + 1) % 3]);
        bool outside = ((v1 - projected) ^ (v2 - projected)) * fnorm < 0.0;
        if (outside)
        {
            nOutside++;
            vec3 ln = v2 - v1;
            double param = std::clamp((ln * (projected - v1)) / ln.len_sq(), 0.0, 1.0);
            vec3 cpt = v2 * param + v1 * (1.0 - param);
            double distSq = (cpt - pt).len_sq();
            if (distSq < bestSqDist)
            {
                closePt = cpt;
                bestSqDist = distSq;
            }
        }

        if (nOutside > 1) break;
    }

    if (nOutside == 0)
    {
        closePt = projected;
        bestSqDist = planeDistSq;
    }
}

mesh::mesh(const mesh& other) : mesh(other.m_vertices.data(), other.num_vertices(), other.m_faces.data(), other.num_faces())
{
}

mesh::mesh(const vec3* verts, size_t nVerts, const mesh_face* faces, size_t nFaces)
    : m_vertEdges(nVerts), m_vertFaces(nVerts), m_faceEdges(nFaces)
{
    m_vertices.reserve(nVerts);
    std::copy(verts, verts + nVerts, std::back_inserter(m_vertices));
    m_faces.reserve(nFaces);
    std::copy(faces, faces + nFaces, std::back_inserter(m_faces));
    compute_cache();
}

mesh::mesh(const double* vertCoords, size_t nVerts, const size_t* faceVertIndices, size_t nFaces)
    : m_vertEdges(nVerts), m_vertFaces(nVerts), m_faceEdges(nFaces)
{
    m_vertices.reserve(nVerts);
    size_t nFlat = nVerts * 3;
    size_t i = 0;
    while (i < nFlat)
    {
        double x = vertCoords[i++];
        double y = vertCoords[i++];
        double z = vertCoords[i++];
        m_vertices.emplace_back(x, y, z);
    }

    m_faces.reserve(nFaces);
    nFlat = nFaces * 3;
    i = 0;
    while (i < nFlat)
    {
        size_t a = faceVertIndices[i++];
        size_t b = faceVertIndices[i++];
        size_t c = faceVertIndices[i++];
        m_faces.emplace_back(a, b, c);
    }

    compute_cache();
}

size_t mesh::num_vertices() const noexcept
{
    return m_vertices.size();
}

size_t mesh::num_faces() const noexcept
{
    return m_faces.size();
}

vec3 mesh::vertex(size_t vi) const
{
    return vi < num_vertices() ? m_vertices[vi] : vec3::unset;
}

mesh_face mesh::face(size_t fi) const
{
    return fi < num_faces() ? m_faces[fi] : mesh_face::unset;
}

vec3 mesh::vertex_normal(size_t vi) const
{
    return vi < num_vertices() ? m_vertexNormals[vi] : vec3::unset;
}

const vec3& mesh::face_normal(size_t fi) const
{
    static const vec3 s_unset = vec3::unset;
    return fi < num_faces() ? m_faceNormals.at(fi) : s_unset;
}

mesh::const_vertex_iterator mesh::vertex_cbegin() const
{
    return m_vertices.cbegin();
}

mesh::const_vertex_iterator mesh::vertex_cend() const
{
    return m_vertices.cend();
}

mesh::const_face_iterator mesh::face_cbegin() const
{
    return m_faces.cbegin();
}

mesh::const_face_iterator mesh::face_cend() const
{
    return m_faces.cend();
}

box3 mesh::bounds() const
{
    box3 b;
    for (const vec3& v : m_vertices)
    {
        b.inflate(v);
    }
    return b;
}

double mesh::face_area(size_t fi) const
{
    return face_area(face(fi));
}

double mesh::area() const
{
    double sum = 0;
    for (const mesh_face& f : m_faces)
    {
        sum += face_area(f);
    }
    return sum;
}

box3 mesh::face_bounds(size_t fi) const
{
    box3 b;
    mesh_face f = m_faces[fi];
    b.inflate(m_vertices[f.a]);
    b.inflate(m_vertices[f.b]);
    b.inflate(m_vertices[f.c]);
    return b;
}

double mesh::volume() const
{
    if (!is_solid())
        return 0.0;

    vec3 refPt = bounds().center();
    std::vector<vec3> joinVectors;
    joinVectors.reserve(num_vertices());
    std::transform(vertex_cbegin(), vertex_cend(), std::back_inserter(joinVectors),
        [&refPt](const vec3 vert) {
            return vert - refPt;
        });

    double total = 0.0;
    for (size_t fi = 0; fi < m_faces.size(); fi++)
    {
        mesh_face face = m_faces[fi];
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

bool mesh::is_solid() const
{
    return m_isSolid;
}

vec3 mesh::centroid() const
{
    return centroid(mesh_centroid_type::vertex_based);
}

vec3 mesh::centroid(const mesh_centroid_type centroid_type) const
{
    switch (centroid_type)
    {
    case mesh_centroid_type::vertex_based:
        return vec3::average(vertex_cbegin(), vertex_cend());
    case mesh_centroid_type::area_based:
        return area_centroid();
    case mesh_centroid_type::volume_based:
        return volume_centroid();
    default:
        return centroid(mesh_centroid_type::vertex_based);
    }
}

bool mesh::contains(const vec3& pt) const
{
    static const auto comparer = [](const std::pair<double, double>& a, const std::pair<double, double>& b) {
        return a.first < b.first;
    };

    box3 b(pt, { pt.x, pt.y, DBL_MAX });
    std::vector<size_t> faces;
    faces.reserve(10);
    m_faceTree.query_box_intersects(b, std::back_inserter(faces));
    vec3 triangles[3];
    vec2 p2(pt);

    std::vector<std::pair<double, double>> hits;
    hits.reserve(faces.size());

    for (size_t fi : faces)
    {
        mesh_face f = m_faces[fi];
        vec3 pts3[3] = { m_vertices[f.a], m_vertices[f.b], m_vertices[f.c] };
        vec2 pts2[3] = { {pts3[0]}, {pts3[1]}, {pts3[2]} };
        double bary[3];
        utils::barycentric_coords(pts2, p2, bary);
        if (!utils::barycentric_within_bounds(bary))
            continue;
        hits.emplace_back(face_normal(fi).z, utils::barycentric_evaluate(bary, pts3).z);
    }

    std::sort(hits.begin(), hits.end(), comparer);
    bool first = true;
    double last = 0;
    size_t count = 0;
    for (const std::pair<double, double>& hit : hits)
    {
        if (first || last * hit.second < 0)
            count++;
        last = hit.second;
        first = false;
    }

    return count % 2;
}

mesh* mesh::clipped_with_plane(const vec3& pt, const vec3& normal) const
{
    vec3 unorm = normal.unit();

    // Calculate vertex distances.
    std::vector<double> vdistances(num_vertices());
    std::transform(vertex_cbegin(), vertex_cend(), vdistances.data(),
        [&pt, &unorm](const vec3& v) {
            return (v - pt) * unorm;
        });

    // Compute edge-plane intersection points.
    std::vector<vec3> edgepts(m_edges.size());
    for (size_t ei = 0; ei < m_edges.size(); ei++)
    {
        const edge_type& edge = m_edges.at(ei);
        double d1 = vdistances[edge.p];
        double d2 = vdistances[edge.q];
        if (d1 * d2 >= 0)
            continue;

        double r = d2 / (d2 - d1);
        edgepts[ei] = (vertex(edge.p) * r) + (vertex(edge.q) * (1.0 - r));
    }

    // Compute vertex enums for all faces.
    std::vector<uint8_t> venums(num_faces());
    std::transform(face_cbegin(), face_cend(), venums.data(),
        [&vdistances](const mesh_face& face) {
            uint8_t mask = 0u;
            if (vdistances[face.a] < 0) mask |= 1 << 0;
            if (vdistances[face.b] < 0) mask |= 1 << 1;
            if (vdistances[face.c] < 0) mask |= 1 << 2;
            return mask;
        });

    // Total number of triangle indices.
    std::vector<size_t> indices;
    size_t nIndices = 0;
    for (const uint8_t venum : venums)
        nIndices += s_clipVertCountTable[venum];
    assert(nIndices % 3 == 0);

    // Copy the indices and vertices.
    std::unordered_map<size_t, size_t, custom_size_t_hash> map;
    map.reserve(nIndices);
    std::vector<vec3> verts;
    verts.reserve(num_vertices());

    indices.reserve(nIndices);
    auto indexIt = std::back_inserter(indices);

    for (size_t fi = 0; fi < m_faces.size(); fi++)
    {
        const mesh_face& face = m_faces.at(fi);
        uint8_t venum = venums[fi];
        const uint8_t* const row = s_clipTriTable[venum].data();
        std::transform(row, row + s_clipVertCountTable[venum], indexIt,
            [this, &map, &verts, &edgepts, &face](const uint8_t vi) {
                decltype(m_edgeIndexMap)::const_iterator match2;
                if (vi > 2)
                {
                    match2 = m_edgeIndexMap.find(face.edge(vi - 3));
                    if (match2 == m_edgeIndexMap.end())
                        throw 1;
                }
                size_t key;
                switch (vi)
                {
                case 0: key = face.a; break;
                case 1: key = face.b; break;
                case 2: key = face.c; break;
                case 3: case 4: case 5:
                    key = match2->second + num_vertices(); break;
                }
                auto match = map.find(key);
                if (match == map.end())
                {
                    size_t vi2 = verts.size();
                    map.emplace(key, vi2);

                    switch (vi)
                    {
                    case 0: verts.push_back(vertex(face.a)); break;
                    case 1: verts.push_back(vertex(face.b)); break;
                    case 2: verts.push_back(vertex(face.c)); break;
                    case 3: case 4: case 5:
                        verts.push_back(edgepts[match2->second]); break;
                    }
                    return vi2;
                }
                return match->second;
            });
    }

    assert(indices.size() == nIndices);
    // Create new mesh with the copied data.
    return new mesh((const double*)verts.data(), verts.size(), indices.data(), nIndices / 3);
}

vec3 mesh::closest_point(const vec3& pt, double searchDist) const
{
    size_t nearestVertIndex = SIZE_MAX;
    m_vertexTree.query_nearest_n(pt, 1, &nearestVertIndex);
    if (nearestVertIndex == SIZE_MAX) // Didn't find the nearest vertex.
        return vec3::unset;

    vec3 closePt = m_vertices[nearestVertIndex];
    double bestDistSq = (pt - closePt).len_sq();

    double vDist = std::sqrt(bestDistSq);
    if (vDist > searchDist) // Closest point not found within search distance.
        return vec3::unset;

    vec3 halfDiag(vDist, vDist, vDist);
    std::vector<size_t> candidates;
    candidates.reserve(32);
    m_faceTree.query_box_intersects(box3(pt - halfDiag, pt + halfDiag), std::back_inserter(candidates));

    for (size_t fi : candidates)
    {
        face_closest_pt(fi, pt, closePt, bestDistSq);
    }
    return closePt;
}

face_edges::face_edges(size_t const indices[3])
    :a(indices[0]), b(indices[1]), c(indices[2])
{
}

face_edges::face_edges(size_t p, size_t q, size_t r)
    :a(p), b(q), c(r)
{
}

void face_edges::set(size_t p, size_t q, size_t r)
{
    a = p;
    b = q;
    c = r;
}

void face_edges::set(size_t i)
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
