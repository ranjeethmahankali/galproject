#pragma once
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "base.h"

constexpr double PLANE_DIST_TOL = 1e-10;

class convex_hull
{
private:
	std::vector<vec3> m_pts;
	size_t m_nPts;
	vec3 m_center;

	std::unordered_map<size_t, tri_face, custom_size_t_hash, std::equal_to<size_t>> m_faces;
	std::unordered_map<index_pair, index_pair, index_pair_hash, std::equal_to<index_pair>> m_edgeFaceMap;
	std::unordered_set<size_t, custom_size_t_hash, std::equal_to<size_t>> m_outsidePts;

	void compute();
	void set_face(tri_face& face);
	tri_face pop_face(size_t index, index_pair edges[3], tri_face adjFaces[3]);
	bool face_visible(const tri_face&, const vec3&) const;
	double face_plane_dist(const tri_face&, const vec3&) const;
	bool get_farthest_pt(const tri_face&, vec3& pt, size_t& ptIndex) const;
	void update_exterior_pts(const std::vector<tri_face>& newFaces, const std::vector<tri_face>& poppedFaces);
	void create_initial_simplex(size_t& faceIndex);
	bool get_face(size_t id, tri_face& face) const;
	bool get_edge_faces(const index_pair& edge, index_pair& faces) const;
	vec3 face_center(const tri_face& face) const;

public:
	convex_hull(double* coords, size_t nPts);

	vec3 get_pt(size_t index) const;
	size_t num_faces() const;
	void copy_faces(int* faceIndices) const;
};

PINVOKE void ConvexHull_Create(double* coords, size_t nPts, int* &faceIndices, int& nFaces);