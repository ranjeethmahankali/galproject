#pragma once
#include "base.h"
#include "rtree.h"
#include <unordered_map>
#include <limits>

typedef index_pair edge_type;
typedef index_pair_hash edge_type_hash;

struct mesh_face
{
    static const mesh_face unset;
    size_t a = SIZE_MAX, b = SIZE_MAX, c = SIZE_MAX;

    mesh_face() = default;
    mesh_face(size_t v1, size_t v2, size_t v3);
    mesh_face(size_t const indices[3]);

    void flip();
    edge_type edge(uint8_t edgeIndex) const;
    bool contains_vertex(size_t vertIndex) const;
    bool is_degenerate() const;
};

struct face_edges
{
    size_t a = SIZE_MAX, b = SIZE_MAX, c = SIZE_MAX;
    
    face_edges() = default;
    face_edges(size_t const indices[3]);
    face_edges(size_t, size_t, size_t);

    void set(size_t, size_t, size_t);
    void set(size_t);
};

enum class mesh_centroid_type
{
    vertex_based = 0,
    area_based,
    volume_based
};

enum class mesh_element
{
    vertex,
    face
};

class mesh
{
    typedef std::vector<vec3>::const_iterator const_vertex_iterator;
    typedef std::vector<mesh_face>::const_iterator const_face_iterator;

private:
    std::vector<vec3> m_vertices;
    std::vector<mesh_face> m_faces;

    /*Maps vertex indices to indices of connected faces.*/
    std::vector<std::vector<size_t>> m_vertFaces;
    /*Maps the vertex indices to the indices of connected edges.*/
    std::vector<std::vector<size_t>> m_vertEdges;
    /*Maps the vertex-index-pair to the index of the edge connecting those vertices.*/
    std::unordered_map<edge_type, size_t, edge_type_hash> m_edgeIndexMap;
    /*Maps the edge index to the pair of connected vertex indices.*/
    std::vector<edge_type> m_edges;
    /*Maps the edge index to the indices of the connected faces.*/
    std::vector<std::vector<size_t>> m_edgeFaces;
    /*Maps the face index to the indices of the 3 edges connected to that face.*/
    std::vector<face_edges> m_faceEdges;

    std::vector<vec3> m_vertexNormals;
    std::vector<vec3> m_faceNormals;
    bool m_isSolid;
    rtree3d m_faceTree;
    rtree3d m_vertexTree;

    void compute_cache();
    void compute_topology();
    void compute_rtrees();
    void compute_normals();
    void add_edge(const mesh_face&, size_t fi, uint8_t, size_t&);
    void add_edges(const mesh_face&, size_t fi);
    double face_area(const mesh_face& f) const;
    void get_face_center(const mesh_face& f, vec3& center) const;
    void check_solid();

    vec3 area_centroid() const;
    vec3 volume_centroid() const;
    const rtree3d& element_tree(mesh_element element) const;

public:
    mesh(const mesh& other);
    mesh(const vec3* verts, size_t nVerts, const mesh_face* faces, size_t nFaces);
    mesh(const double* vertCoords, size_t nVerts, const size_t* faceVertIndices, size_t nFaces);
    
    size_t num_vertices() const;
    size_t num_faces() const;
    vec3 vertex(size_t vi) const;
    mesh_face face(size_t fi) const;
    vec3 vertex_normal(size_t vi) const;
    vec3 face_normal(size_t fi) const;
    mesh::const_vertex_iterator vertex_cbegin() const;
    mesh::const_vertex_iterator vertex_cend() const;
    mesh::const_face_iterator face_cbegin() const;
    mesh::const_face_iterator face_cend() const;

    box3 bounds() const;
    double face_area(size_t fi) const;
    double area() const;
    box3 face_bounds(size_t fi) const;

    double volume() const;
    bool is_solid() const;
    vec3 centroid() const;
    vec3 centroid(const mesh_centroid_type centroid_type) const;

    bool contains(const vec3& pt) const;

    mesh* clipped_with_plane(const vec3& pt, const vec3& norm) const;

    template <typename size_t_inserter> void query_box(const box3& box, size_t_inserter inserter, mesh_element element) const
    {
        element_tree(element).query_box_intersects(box, inserter);
    };

    template <typename size_t_inserter> void query_sphere(const vec3& center, double radius, size_t_inserter inserter, mesh_element element) const
    {
        element_tree(element).query_by_distance(center, radius, inserter);
    };
};

PINVOKE void Mesh_GetData(mesh const* meshPtr, double*& vertices, int& nVerts, int*& faces, int& nFaces) noexcept;

PINVOKE mesh* Mesh_Create(double const * vertices, int nVerts, int const* faces, int nFaces) noexcept;

PINVOKE void Mesh_Delete(mesh const* meshPtr) noexcept;

PINVOKE double Mesh_Volume(mesh const* meshPtr) noexcept;

PINVOKE void Mesh_Centroid(mesh const* meshPtr, mesh_centroid_type type, double& x, double& y, double &z) noexcept;

PINVOKE void Mesh_QueryBox(mesh const* meshptr, double const* bounds, int32_t*& retIndices, int32_t& numIndices, mesh_element element) noexcept;

PINVOKE void Mesh_QuerySphere(mesh const* meshptr, double cx, double cy, double cz, double radius,
    int32_t*& retIndices, int32_t& numIndices, mesh_element element) noexcept;

PINVOKE bool Mesh_ContainsPoint(mesh const* meshptr, double x, double y, double z);

PINVOKE mesh* Mesh_ClipWithPlane(mesh const* meshptr, double* pt, double* norm);