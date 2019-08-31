#pragma once
#include <limits>
#include <vector>

#define PINVOKE extern "C" __declspec(dllexport)
#define doubleMaxValue std::numeric_limits<double>::max()
#define doubleMinValue std::numeric_limits<double>::min()

struct vec3 {
	double x = 0, y = 0, z = 0;

	static const vec3 unset;
	static const vec3 zero;

	vec3(double, double, double);
	vec3(const vec3& v);
	vec3();

	vec3 operator +(const vec3&) const;
	vec3 operator -(const vec3&) const;
	double operator *(const vec3&) const;
	vec3 operator ^(const vec3&) const;

	vec3 operator *(double) const;
	vec3 operator /(double) const;

	bool operator ==(const vec3&) const;
	bool operator !=(const vec3&) const;

	vec3 operator +=(const vec3&);
	vec3 operator -=(const vec3&);
	vec3 operator /=(double);
	vec3 operator *=(double);

	vec3 operator -() const;

	double len_sq() const;
	double len() const;

	void copy(double* dest, size_t& pos) const;
	bool is_zero() const;
	bool is_valid() const;
	vec3 unit() const;
	void reverse();

	static vec3 sum(const std::vector<vec3>& vecs);
	static vec3 average(const std::vector<vec3>& vecs);
};

struct index_pair {
	size_t p, q;

	bool operator ==(const index_pair&) const;
	bool operator !=(const index_pair&) const;

	index_pair(size_t i, size_t j);
	index_pair();

	size_t hash() const;
	void unset(size_t);
	bool add(size_t);
	bool contains(size_t) const;
};

struct tri_face {
	size_t id;
	size_t a, b, c;
	vec3 normal;

	tri_face();
	tri_face(size_t i, size_t v1, size_t v2, size_t v3);

	bool is_valid();
	void flip();
	index_pair edge(char edgeIndex) const;
	bool contains_vertex(size_t vertIndex) const;
};

PINVOKE void Unsafe_ReleaseInt(int* arr, bool isArray);
PINVOKE void Unsafe_ReleaseDouble(double* arr, bool isArray);