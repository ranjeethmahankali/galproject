#pragma once
#include <limits>
#include <vector>
#include <algorithm>

#define PINVOKE extern "C" __declspec(dllexport)
// The number with the largest absolute value that can be represented by the double datatype.
#define doubleMaxValue std::numeric_limits<double>::max()
// The number with the smallest absolute value that can be represented by the double datatype.
#define doubleMinValue std::numeric_limits<double>::min()

struct vec2
{
    double x = 0, y = 0;
    
    static const vec2 unset;
    static const vec2 zero;

    vec2(double, double);
    vec2(const vec2& v);
    vec2();

    vec2 operator +(const vec2&) const;
    vec2 operator -(const vec2&) const;
    double operator *(const vec2&) const;
    vec2 operator *(double) const;
    vec2 operator /(double) const;

    bool operator ==(const vec2&) const;
    bool operator !=(const vec2&) const;

    void operator +=(const vec2&);
    void operator -=(const vec2&);
    void operator /=(double);
    void operator *=(double);

    vec2 operator -() const;

    double len_sq() const;
    double len() const;

    void copy(double* dest, size_t& pos) const;
    void copy(double dest[2]) const;
    bool is_zero() const;
    bool is_valid() const;
    vec2 unit() const;
    void reverse();
    void set(double, double);
    void set(const vec2&);

    static vec2 sum(const std::vector<vec2>&);
    static vec2 average(const std::vector<vec2>&);
    static vec2 min_coords(const vec2&, const vec2&);
    static vec2 max_coords(const vec2&, const vec2&);
};

struct vec3
{
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

	void operator +=(const vec3&);
	void operator -=(const vec3&);
	void operator /=(double);
	void operator *=(double);

	vec3 operator -() const;

	double len_sq() const;
	double len() const;

	void copy(double* dest, size_t& pos) const;
	void copy(double dest[3]) const;
	bool is_zero() const;
	bool is_valid() const;
	vec3 unit() const;
	void reverse();
    void set(double, double, double);
    void set(const vec3&);

	static vec3 sum(const std::vector<vec3>& vecs);
	static vec3 average(const std::vector<vec3>& vecs);
    static vec3 min_coords(const vec3&, const vec3&);
    static vec3 max_coords(const vec3&, const vec3&);
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
	static const tri_face unset;

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

struct index_pair_hash {
	size_t operator ()(const index_pair&) const noexcept;
};

struct custom_size_t_hash {
	size_t operator ()(const size_t&) const noexcept;
};

struct box3
{
    static const box3 empty;
    vec3 min, max;

    box3();
    box3(const vec3& min, const vec3& max);

    vec3 diagonal() const;
    void inflate(const vec3&);
    void inflate(double);
    void deflate(double);
    bool contains(const vec3&) const;
    bool contains(const box3&) const;
    bool intersects(const box3&) const;

    static box3 init(const vec3&, const vec3&);
};

PINVOKE void ReleaseInt(int* arr, bool isArray);
PINVOKE void ReleaseDouble(double* arr, bool isArray);