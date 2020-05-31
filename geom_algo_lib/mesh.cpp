#include "mesh.h"
#define _USE_MATH_DEFINES
#include <math.h>

const mesh_face mesh_face::unset = mesh_face(-1, -1, -1);

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
    for (size_t vi = 0; vi < nVertices; vi++)
    {
        m_vertFaceMap.insert(std::make_pair(vi, std::vector<size_t>()));
        m_vertEdgeMap.insert(std::make_pair(vi, std::vector<size_t>()));
    }

    size_t curEi = 0;
    for (size_t fi = 0; fi < m_faces.size(); fi++)
    {
        mesh_face f = m_faces[fi];
        m_vertFaceMap[f.a].push_back(fi);
        m_vertFaceMap[f.b].push_back(fi);
        m_vertFaceMap[f.c].push_back(fi);

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
    for (auto const& pair : m_vertFaceMap)
    {
        faceNormals.clear();
        faceNormals.reserve(pair.second.size());
        std::transform(pair.second.cbegin(), pair.second.cend(), std::back_inserter(faceNormals),
            [this](const size_t fi) { return m_faceNormals[fi]; });
        m_vertexNormals[pair.first].set(vec3::average(faceNormals.cbegin(), faceNormals.cend()).unit());
    }
}

void mesh::add_edge(const mesh_face& f, size_t fi, uint8_t fei, size_t& newEi)
{
    // This should be equal to the number edges, and serve as the index of the next edge being added.
    static size_t nextEdgeIdx = 0;

    edge_type e = f.edge(fei);
    auto eMatch = m_edgeIndexMap.find(e);
    if (eMatch == m_edgeIndexMap.end())
    {
        newEi = nextEdgeIdx++;
        m_edgeIndexMap.insert(std::make_pair(e, newEi));

        m_vertEdgeMap[e.p].push_back(newEi);
        m_vertEdgeMap[e.q].push_back(newEi);
        m_edgeVertMap.insert(std::make_pair(newEi, e));
        m_edgeFaceMap.insert(std::make_pair(newEi, std::vector<size_t>()));
    }
    else
    {
        newEi = eMatch->second;
    }

    m_edgeFaceMap[newEi].push_back(fi);
}

void mesh::add_edges(const mesh_face& f, size_t fi)
{
    size_t indices[3];
    for (uint8_t fei = 0; fei < 3; fei++)
    {
        add_edge(f, fi, fei, indices[fei]);
    }
    m_faceEdgeMap.insert(std::make_pair(fi, face_edges(indices)));
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
    for (const auto& edge : m_edgeFaceMap)
    {
        if (edge.second.size() != 2)
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

mesh::mesh(const mesh& other) : mesh(other.vertex_cbegin(), other.vertex_cend(), other.face_cbegin(), other.face_cend())
{
}

mesh::mesh(const vec3* verts, size_t nVerts, const mesh_face* faces, size_t nFaces)
{
    m_vertices.reserve(nVerts);
    std::copy(verts, verts + nVerts, std::back_inserter(m_vertices));
    m_faces.reserve(nFaces);
    std::copy(faces, faces + nFaces, std::back_inserter(m_faces));
    compute_cache();
}

mesh::mesh(const double* vertCoords, size_t nVerts, const size_t* faceVertIndices, size_t nFaces)
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

size_t mesh::num_vertices() const
{
    return m_vertices.size();
}

size_t mesh::num_faces() const
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

vec3 mesh::face_normal(size_t fi) const
{
    return fi < num_faces() ? m_faceNormals[fi] : vec3::unset;
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

PINVOKE void Mesh_GetData(mesh const* meshPtr, double*& vertices, int& nVerts, int*& faces, int& nFaces) noexcept
{
    if (vertices || faces)
    {
        return;
    }

    vertices = new double[meshPtr->num_vertices() * 3];
    faces = new int[meshPtr->num_faces() * 3];
    size_t i = 0;
    for (auto vi = meshPtr->vertex_cbegin(); vi != meshPtr->vertex_cend(); vi++)
    {
        vec3 v = *vi;
        v.copy(vertices, i);
    }

    i = 0;
    for (auto fi = meshPtr->face_cbegin(); fi != meshPtr->face_cend(); fi++)
    {
        mesh_face f = *fi;
        faces[i++] = (int)f.a;
        faces[i++] = (int)f.b;
        faces[i++] = (int)f.c;
    }

    nVerts = (int)meshPtr->num_vertices();
    nFaces = (int)meshPtr->num_faces();
}

PINVOKE mesh* Mesh_Create(double const* vertices, int numVerts, int const* faceIndices, int numFaces) noexcept
{
    size_t nVerts = (size_t)numVerts;
    size_t nFaces = (size_t)numFaces;

    std::vector<vec3> verts;
    verts.reserve(nVerts);
    size_t nCoords = nVerts * 3;
    size_t ci = 0;
    while (ci < nCoords)
    {
        double x = vertices[ci++];
        double y = vertices[ci++];
        double z = vertices[ci++];
        verts.emplace_back(x, y, z);
    }

    std::vector<mesh_face> faces;
    faces.reserve(nFaces); 
    size_t nIndices = nFaces * 3;
    size_t ii = 0;
    while (ii < nIndices)
    {
        size_t a = (size_t)faceIndices[ii++];
        size_t b = (size_t)faceIndices[ii++];
        size_t c = (size_t)faceIndices[ii++];
        faces.emplace_back(a, b, c);
    }

    return new mesh(verts.cbegin(), verts.cend(), faces.cbegin(), faces.cend());
}

PINVOKE void Mesh_Delete(mesh const* meshPtr) noexcept
{
    if (meshPtr)
        delete meshPtr;
}

PINVOKE double Mesh_Volume(mesh const* meshPtr) noexcept
{
    return meshPtr->volume();
}

PINVOKE void Mesh_Centroid(mesh const* meshPtr, mesh_centroid_type type, double& x, double& y, double& z) noexcept
{
    vec3 center = meshPtr->centroid(type);
    x = center.x;
    y = center.y;
    z = center.z;
}

PINVOKE void Mesh_QueryBox(mesh const* meshptr, double const* bounds, int32_t*& retIndices, int32_t& numIndices, mesh_element element) noexcept
{
    box3 box(vec3(bounds), vec3(bounds + 3));
    box3 mbox = meshptr->bounds();
    size_t nIndices = meshptr->num_faces();
    size_t estimate = (size_t)std::ceil(std::min(1., (box.volume() / mbox.volume()))) * nIndices;
    std::vector<size_t> indices;
    indices.reserve(estimate);
    auto inserter = std::back_inserter(indices);
    meshptr->query_box(box, inserter, element);

    retIndices = new int32_t[indices.size()];
    numIndices = (int32_t)indices.size();
    std::transform(indices.cbegin(), indices.cend(), retIndices, [](const size_t fi) { return (int32_t)fi; });
}

void Mesh_QuerySphere(mesh const* meshptr, double cx, double cy, double cz, double radius, int32_t*& retIndices, int32_t& numIndices, mesh_element element) noexcept
{
    vec3 center(cx, cy, cz);
    box3 mbox = meshptr->bounds();
    size_t nIndices = meshptr->num_faces();
    size_t estimate = (size_t)std::ceil(std::min(1., ((4.0 * M_PI * std::pow(radius, 3) / 3.0) / mbox.volume()))) * nIndices;
    std::vector<size_t> indices;
    indices.reserve(estimate);
    auto inserter = std::back_inserter(indices);
    meshptr->query_sphere(center, radius, inserter, element);

    retIndices = new int32_t[indices.size()];
    numIndices = (int32_t)indices.size();
    std::transform(indices.cbegin(), indices.cend(), retIndices, [](const size_t fi) { return (int32_t)fi; });
}

bool Mesh_ContainsPoint(mesh const* meshptr, double x, double y, double z)
{
    return meshptr->contains({ x, y, z });
}
