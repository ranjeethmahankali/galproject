#pragma once
#include "base.h"
#include "rtree.h"
#include <unordered_map>

typedef index_pair edge_type;
typedef index_pair_hash edge_type_hash;

struct mesh_face
{
    static const mesh_face unset;
    size_t a, b, c;
    
    mesh_face(size_t v1, size_t v2, size_t v3);
    mesh_face(size_t indices[3]);

    void flip();
    edge_type edge(uint8_t edgeIndex) const;
    bool contains_vertex(size_t vertIndex) const;
    bool is_degenerate() const;
};

struct face_edges
{
    size_t a, b, c;
    
    face_edges(size_t indices[3]);
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

class mesh
{
    typedef std::vector<vec3>::const_iterator const_vertex_iterator;
    typedef std::vector<mesh_face>::const_iterator const_face_iterator;

private:
    std::vector<vec3> m_vertices;
    std::vector<mesh_face> m_faces;

    /*Maps vertex indices to indices of connected faces.*/
    std::unordered_map<size_t, std::vector<size_t>, custom_size_t_hash> m_vertFaceMap;
    /*Maps the vertex indices to the indices of connected edges.*/
    std::unordered_map<size_t, std::vector<size_t>, custom_size_t_hash> m_vertEdgeMap;
    /*Maps the vertex-index-pair to the index of the edge connecting those vertices.*/
    std::unordered_map<edge_type, size_t, edge_type_hash> m_edgeIndexMap;
    /*Maps the edge index to the pair of connected vertex indices.*/
    std::unordered_map<size_t, edge_type, custom_size_t_hash> m_edgeVertMap;
    /*Maps the edge index to the indices of the connected faces.*/
    std::unordered_map<size_t, std::vector<size_t>, custom_size_t_hash> m_edgeFaceMap;
    /*Maps the face index to the indices of the 3 edges connected to that face.*/
    std::unordered_map<size_t, face_edges, custom_size_t_hash> m_faceEdgeMap;

    std::vector<vec3> m_vertexNormals;
    std::vector<vec3> m_faceNormals;

    rtree3d m_faceTree;
    rtree3d m_vertexTree;

    void compute_cache();
    void compute_topology();
    void compute_rtrees();
    void compute_normals();
    void add_edge(const mesh_face&, size_t fi, uint8_t, size_t&);
    void add_edges(const mesh_face&, size_t fi);
    double face_area(const mesh_face& f) const;

public:
    mesh(const mesh& other);
    mesh(const vec3* verts, size_t nVerts, const mesh_face* faces, size_t nFaces);
    mesh(const double* vertCoords, size_t nVerts, const size_t* faceVertIndices, size_t nFaces);
    template<typename vec3_iter, typename mesh_face_iter> mesh(
        vec3_iter vbegin, vec3_iter vend,
        mesh_face_iter fbegin, mesh_face_iter fend)
        :m_vertices(vbegin, vend), m_faces(fbegin, fend)
    {
        compute_cache();
    };
    
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
};

PINVOKE void Mesh_GetData(mesh* meshPtr, double*& vertices, int& nVerts, int*& faces, int& nFaces) noexcept;

PINVOKE mesh* Mesh_Create(double* vertices, int nVerts, int* faces, int nFaces) noexcept;

PINVOKE void Mesh_Delete(mesh* meshPtr) noexcept;

PINVOKE double Mesh_Volume(mesh* meshPtr) noexcept;