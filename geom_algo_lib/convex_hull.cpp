#include "convex_hull.h"

convex_hull::convex_hull(double* coords, size_t nPts)
{
	m_pts.reserve(nPts);
	for (size_t i = 0; i < nPts; i++)
	{
		m_pts.push_back(vec3(coords[3 * i], coords[3 * i + 1], coords[3 * i + 2]));
		m_outsidePts.insert(i);
	}

	m_nPts = nPts;
	compute();
}

vec3 convex_hull::get_pt(size_t index) const
{
	return index < 0 || index > m_nPts - 1 ? vec3::unset : m_pts[index];
}

size_t convex_hull::num_faces() const
{
	return m_faces.size();
}

void convex_hull::copy_faces(int* faceIndices) const
{
	int i = 0;
	for (auto const& pair : m_faces) {
		faceIndices[i++] = pair.second.a;
		faceIndices[i++] = pair.second.b;
		faceIndices[i++] = pair.second.c;
	}
}

void convex_hull::compute()
{
	size_t curFaceId = 0;
	create_initial_simplex(curFaceId);
	std::queue<size_t> faceQ;
	for (const auto& f : m_faces) {
		faceQ.push(f.first);
	}

	size_t fi, fpi;
	tri_face curFace, pFace, newFace;
	tri_face adjFaces[3];
	index_pair edges[3];
	vec3 farPt;
	std::queue<size_t> popQ;
	std::vector<index_pair> horizonEdges;
	std::vector<tri_face> poppedFaces, newFaces;

	while (!faceQ.empty()) {
		fi = faceQ.front();
		faceQ.pop();
		if (!get_face(fi, curFace) || !get_farthest_pt(curFace, farPt, fpi)) {
			continue;
		}
		popQ.push(fi);

		horizonEdges.clear();
		poppedFaces.clear();
		while (!popQ.empty()) {
			pFace = pop_face(popQ.front(), edges, adjFaces);
			popQ.pop();

			if (!pFace.is_valid()) {
				continue;
			}

			poppedFaces.push_back(pFace);

			for (size_t i = 0; i < 3; i++)
			{
				if (!adjFaces[i].is_valid()) {
					continue;
				}
				if (face_visible(adjFaces[i], farPt)) {
					popQ.push(adjFaces[i].id);
				}
				else {
					horizonEdges.push_back(edges[i]);
				}
			}
		}

		newFaces.clear();
		newFaces.reserve(horizonEdges.size());
		for (const index_pair& he : horizonEdges) {
			newFace = tri_face(curFaceId++, fpi, he.p, he.q);
			set_face(newFace);
			faceQ.push(newFace.id);
			newFaces.push_back(newFace);
		}

		update_exterior_pts(newFaces, poppedFaces);
	}
}

void convex_hull::set_face(tri_face& face)
{
	face.normal = ((m_pts[face.b] - m_pts[face.a]) ^ (m_pts[face.c] - m_pts[face.a])).unit();
	if (face_visible(face, m_center)) {
		face.flip();
	}

	m_faces.insert_or_assign(face.id, face);

	for (char ei = 0; ei < 3; ei++)
	{
		if (!m_edgeFaceMap[face.edge(ei)].add(face.id)) {
			throw "Failed to add face to the edge map.";
		}
	}
}

tri_face convex_hull::pop_face(size_t id, index_pair edges[3], tri_face adjFaces[3])
{
	tri_face face;
	if (get_face(id, face)) {
		m_faces.erase(id);
		index_pair edge, fPair;
		size_t adjFid;
		for (char ei = 0; ei < 3; ei++)
		{
			edge = face.edge(ei);
			edges[ei] = edge;
			if (!get_edge_faces(edge, fPair) || !fPair.contains(id)) {
				adjFaces[ei] = tri_face::unset;
				continue;
			}
			fPair.unset(id);
			m_edgeFaceMap[edge] = fPair;
			adjFid = fPair.p == -1 ? fPair.q : fPair.p;
			if (!get_face(adjFid, adjFaces[ei])) {
				adjFaces[ei] = tri_face::unset;
			}
		}
	}

	return face;
}

bool convex_hull::face_visible(const tri_face& face, const vec3& pt)
{
	return face.normal.is_valid() ? face_plane_dist(face, pt) > PLANE_DIST_TOL : false;
}

double convex_hull::face_plane_dist(const tri_face& face, const vec3& pt)
{
	return (pt - m_pts[face.a]) * face.normal;
}

bool convex_hull::get_farthest_pt(const tri_face& face, vec3& pt, size_t& ptIndex)
{
	ptIndex = -1;
	pt = vec3::unset;
	double dMax = PLANE_DIST_TOL, dist;
	for (const size_t& i : m_outsidePts) {
		dist = face_plane_dist(face, m_pts[i]);
		if (face.contains_vertex(i)) {
			continue;
		}
		if (dist > dMax) {
			dMax = dist;
			ptIndex = i;
			pt = m_pts[i];
		}
	}

	return ptIndex != -1;
}

void convex_hull::update_exterior_pts(const std::vector<tri_face>& newFaces, const std::vector<tri_face>& poppedFaces)
{
	bool outside;
	vec3 testPt;
	std::vector<size_t> remove, check;
	for (const size_t& opi : m_outsidePts) {
		outside = false;
		testPt = m_pts[opi];
		for (const tri_face& face : poppedFaces) {
			if (face.contains_vertex(opi)) {
				remove.push_back(opi);
				break;
			}
			if (face_visible(face, testPt)) {
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
		testPt = m_pts[ci];
		for (const tri_face& newFace : newFaces) {
			if (face_visible(newFace, testPt)) {
				outside = true;
				break;
			}
		}

		if (!outside) {
			remove.push_back(ci);
		}
	}

	for (const size_t& ri : remove) {
		m_outsidePts.erase(ri);
	}
}

void convex_hull::create_initial_simplex(size_t& faceIndex)
{
	size_t best[4];
	if (m_nPts < 4) {
		throw "Failed to create the initial simplex";
	}
	else if (m_nPts == 4) {
		for (size_t i = 0; i < 4; i++)
		{
			best[i] = i;
		}
	}
	else {
		double extremes[6];
		for (size_t ei = 0; ei < 6; ei++)
		{
			extremes[ei] = ei % 2 == 0 ? doubleMaxValue : doubleMinValue;
		}

		size_t bounds[6] = { -1, -1, -1, -1, -1, -1 };
		double coords[3];
		for (size_t pi = 0; pi < m_nPts; pi++)
		{
			m_pts[pi].copy(coords);
			for (size_t ei = 0; ei < 6; ei++)
			{
				if (ei % 2 == 0 && extremes[ei] > coords[ei / 2]) {
					extremes[ei] = coords[ei / 2];
					bounds[ei] = pi;
				}
				else if (ei % 2 == 1 && extremes[ei] < coords[ei / 2]) {
					extremes[ei] = coords[ei / 2];
					bounds[ei] = pi;
				}
			}
		}

		vec3 pt;
		double maxD = doubleMinValue, dist;
		for (size_t i = 0; i < 6; i++)
		{
			pt = m_pts[bounds[i]];
			for (size_t j = 0; j < 6; j++)
			{
				dist = (pt - m_pts[bounds[j]]).len_sq();
				if (dist > maxD) {
					best[0] = bounds[i];
					best[1] = bounds[j];
					maxD = dist;
				}
			}
		}

		if (maxD <= 0) {
			throw "Failed to create the initial simplex";
		}

		maxD = doubleMinValue;
		vec3 ref = m_pts[best[0]];
		vec3 uDir = (m_pts[best[1]] - ref).unit();
		for (size_t pi = 0; pi < m_nPts; pi++)
		{
			dist = ((m_pts[pi] - ref) - uDir * (uDir * (m_pts[pi] - ref))).len_sq();
			if (dist > maxD) {
				best[2] = pi;
				maxD = dist;
			}
		}

		if (maxD <= 0) {
			throw "Failed to create the initial simplex";
		}

		maxD = doubleMinValue;
		uDir = ((m_pts[best[1]] - ref) ^ (m_pts[best[2]] - ref)).unit();
		for (size_t pi = 0; pi < m_nPts; pi++)
		{
			dist = abs(uDir * (m_pts[pi] - ref));
			if (dist > maxD) {
				best[3] = pi;
				maxD = dist;
			}
		}

		if (maxD <= 0) {
			throw "Failed to create the initial simplex";
		}
	}

	tri_face simplex[4];
	simplex[0] = tri_face(faceIndex++, best[0], best[1], best[2]);
	simplex[1] = tri_face(faceIndex++, best[0], best[2], best[3]);
	simplex[2] = tri_face(faceIndex++, best[1], best[2], best[3]);
	simplex[3] = tri_face(faceIndex++, best[0], best[1], best[3]);

	m_center = vec3::zero;
	for (size_t i = 0; i < 4; i++)
	{
		m_center += m_pts[best[i]];
	}
	m_center /= 4;

	for (size_t i = 0; i < 4; i++)
	{
		if (!simplex[i].is_valid()) {
			continue;
		}
		set_face(simplex[i]);
	}

	std::vector<size_t> removePts;
	bool outside;
	for (const size_t& opi : m_outsidePts) {
		outside = false;
		for (size_t i = 0; i < 4; i++)
		{
			if (simplex[i].contains_vertex(opi)) {
				removePts.push_back(opi);
				break;
			}
			if (face_visible(simplex[i], get_pt(opi))) {
				outside = true;
				break;
			}
		}

		if (!outside) {
			removePts.push_back(opi);
		}
	}

	for (const size_t& ri : removePts) {
		m_outsidePts.erase(ri);
	}
}

bool convex_hull::get_face(size_t id, tri_face& face)
{
	auto match = m_faces.find(id);
	if (match != m_faces.end()) {
		face = match->second;
		return true;
	}
	return false;
}

bool convex_hull::get_edge_faces(const index_pair& edge, index_pair& faces)
{
	auto match = m_edgeFaceMap.find(edge);
	if (match != m_edgeFaceMap.end()) {
		faces = match->second;
		return true;
	}
	return false;
}

vec3 convex_hull::face_center(const tri_face& face)
{
	return (m_pts[face.a] + m_pts[face.b] + m_pts[face.c]) / 3;
}

PINVOKE void ConvexHull_Create(double* coords, size_t nPts, int*& faceIndices, int& nFaces)
{
	convex_hull hull(coords, nPts);
	nFaces = hull.num_faces();
	faceIndices = new int[hull.num_faces() * 3];
	hull.copy_faces(faceIndices);
}
