#pragma once
#include <limits>


#define PINVOKE extern "C" __declspec(dllexport)
#define doubleMaxValue std::numeric_limits<double>::max()
#define doubleMinValue std::numeric_limits<double>::min()

struct vec3 {
	double x, y, z;

	static const vec3 unset;
	static const vec3 zero;

	vec3(double, double, double);
	vec3();

	vec3 operator +(const vec3&) const;
	vec3 operator -(const vec3&) const;
	double operator *(const vec3&) const;
	vec3 operator ^(const vec3&) const;

	vec3 operator *(const double&) const;
	vec3 operator /(const double&) const;

	bool operator ==(const vec3&) const;
	bool operator !=(const vec3&) const;

	vec3 operator +=(const vec3&);
	vec3 operator -=(const vec3&);
	vec3 operator /=(const double&);
	vec3 operator *=(const double&);

	vec3 operator -() const;

	double len_sq() const;
	double len() const;

	void copy(double* dest, size_t& pos) const;
	void copy(double dest[3]);
	bool is_zero() const;
	bool is_valid() const;
	vec3 unit() const;

	static vec3 sum(vec3* vecs, size_t nVecs);
	static vec3 average(vec3* vecs, size_t nVecs);
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
	tri_face(size_t id, size_t v1, size_t v2, size_t v3);

	bool is_valid();
	void flip();
	index_pair edge(char edgeIndex);
};

PINVOKE void Unsafe_ReleaseInt(int* arr, bool isArray);
PINVOKE void Unsafe_ReleaseDouble(double* arr, bool isArray);