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
    void copy(double(&dest)[2]) const;
    bool is_zero() const;
    bool is_valid() const;
    vec2 unit() const;
    void reverse();
    void set(double, double);
    void set(const vec2&);

    template <typename vec3_iter>
    static vec2 sum(vec3_iter first, vec3_iter last)
    {
        vec2 s = vec2::zero;
        for (vec3_iter vi = first; vi != last; vi++)
        {
            s += *vi;
        }
        return s;
    };

    template <typename vec3_iter>
    static vec2 average(vec3_iter first, vec3_iter last)
    {
        vec2 a = vec2::zero;
        double n = 0;
        for (vec3_iter vi = first; vi != last; vi++)
        {
            a += vi;
            n += 1.0;
        }
        return a / n;
    };

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
	void copy(double (&dest)[3]) const;
	bool is_zero() const;
	bool is_valid() const;
	vec3 unit() const;
	void reverse();
    void set(double, double, double);
    void set(const vec3&);

    template <typename vec3_iter>
    static vec3 sum(vec3_iter first, vec3_iter last)
    {
        vec3 s = vec3::zero;
        for (vec3_iter vi = first; vi != last; vi++)
        {
            s += *vi;
        }
        return s;
    };

    template <typename vec3_iter>
    static vec3 average(vec3_iter first, vec3_iter last)
    {
        vec3 a = vec3::zero;
        double n = 0;
        for (vec3_iter vi = first; vi != last; vi++)
        {
            a += *vi;
            n += 1.0;
        }
        return a / n;
    };

    static vec3 min_coords(const vec3&, const vec3&);
    static vec3 max_coords(const vec3&, const vec3&);
};

struct box3
{
    static const box3 empty;
    vec3 min, max;

    box3();
    box3(const vec3& min, const vec3& max);
    box3(const vec3& pt);
    box3(const vec3* points, size_t nPoints);

    vec3 diagonal() const;
    void inflate(const vec3&);
    void inflate(double);
    void deflate(double);
    bool contains(const vec3&) const;
    bool contains(const box3&) const;
    bool intersects(const box3&) const;
    vec3 center() const;

    static box3 init(const vec3&, const vec3&);
};

struct box2
{
    static const box2 empty;
    vec2 min, max;

    box2();
    box2(const vec2& pt);
    box2(const vec2&, const vec2&);
    box2(const vec2* points, size_t nPoints);

    vec2 diagonal() const;
    void inflate(const vec2&);
    void inflate(double);
    void deflate(double);
    bool contains(const vec2&) const;
    bool contains(const box2&) const;
    bool intersects(const box2&) const;
    vec2 center() const;

    static box2 init(const vec2&, const vec2&);
};

struct index_pair {
	size_t p, q;

	bool operator ==(const index_pair&) const;
	bool operator !=(const index_pair&) const;

	index_pair(size_t i, size_t j);
	index_pair();

    void set(size_t, size_t);
	size_t hash() const;
	void unset(size_t);
	bool add(size_t);
	bool contains(size_t) const;
};

struct index_pair_hash {
	size_t operator ()(const index_pair&) const noexcept;
};

struct custom_size_t_hash {
	size_t operator ()(const size_t&) const noexcept;
};

PINVOKE void ReleaseInt(int* arr, bool isArray);
PINVOKE void ReleaseDouble(double* arr, bool isArray);