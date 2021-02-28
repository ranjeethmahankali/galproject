#pragma once
#include <algorithm>
#include <limits>
#include <vector>

// The number with the largest absolute value that
// can be represented by the double datatype.
constexpr double DBL_MAX_VAL = std::numeric_limits<double>::max();
// The number with the smallest absolute value that can be represented by the double datatype.
constexpr double DBL_MIN_VAL = std::numeric_limits<double>::min();

struct vec3;
struct vec2
{
    double x = 0, y = 0;

    static const vec2 unset;
    static const vec2 zero;

    vec2(double, double);
    vec2(const vec2 &v);
    vec2(const vec3 &v);
    vec2();

    vec2 operator+(const vec2 &) const;
    vec2 operator-(const vec2 &) const;
    double operator*(const vec2 &) const;
    vec2 operator*(double) const;
    vec2 operator/(double) const;

    bool operator==(const vec2 &) const;
    bool operator!=(const vec2 &) const;

    void operator+=(const vec2 &);
    void operator-=(const vec2 &);
    void operator/=(double);
    void operator*=(double);

    vec2 operator-() const;

    double len_sq() const;
    double len() const;

    void copy(double *dest, size_t &pos) const;
    void copy(double (&dest)[2]) const;
    bool is_zero() const;
    bool is_valid() const;
    vec2 unit() const;
    void reverse();
    void set(double, double);
    void set(const vec2 &);

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

    static vec2 min_coords(const vec2 &, const vec2 &);
    static vec2 max_coords(const vec2 &, const vec2 &);
};

struct vec3
{
    double x = 0, y = 0, z = 0;

    static const vec3 unset;
    static const vec3 zero;

    vec3(double, double, double);
    vec3(const vec3 &v);
    vec3(const double *const coords);
    vec3();

    vec3 operator+(const vec3 &) const;
    vec3 operator-(const vec3 &) const;
    double operator*(const vec3 &) const;
    vec3 operator^(const vec3 &) const;

    vec3 operator*(double) const;
    vec3 operator/(double) const;

    bool operator==(const vec3 &) const;
    bool operator!=(const vec3 &) const;

    void operator+=(const vec3 &);
    void operator-=(const vec3 &);
    void operator/=(double);
    void operator*=(double);

    vec3 operator-() const;

    double len_sq() const;
    double len() const;

    void copy(double *dest, size_t &pos) const;
    void copy(double (&dest)[3]) const;
    bool is_zero() const;
    bool is_valid() const;
    vec3 unit() const;
    void reverse();
    void set(double, double, double);
    void set(const vec3 &);

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
        return n == 0 ? vec3::unset : (a / n);
    };

    template <typename vec3_iter, typename double_iter>
    static vec3 weighted_average(vec3_iter vFirst, vec3_iter vLast, double_iter wFirst, double_iter wLast)
    {
        vec3 sum = vec3::zero;
        double wSum = 0;
        vec3_iter vIter = vFirst;
        double_iter wIter = wFirst;
        while (vIter != vLast && wIter != wLast)
        {
            double w = *wIter;
            vec3 v = *vIter;
            sum += v * w;
            wSum += w;
            vIter++;
            wIter++;
        }
        return wSum == 0 ? vec3::unset : (sum / wSum);
    };

    static vec3 min_coords(const vec3 &, const vec3 &);
    static vec3 max_coords(const vec3 &, const vec3 &);
};

struct box3
{
    static const box3 empty;
    vec3 min, max;

    box3();
    box3(const vec3 &min, const vec3 &max);
    box3(const vec3 &pt);
    box3(const vec3 *points, size_t nPoints);

    vec3 diagonal() const;
    void inflate(const vec3 &);
    void inflate(double);
    void deflate(double);
    bool contains(const vec3 &) const;
    bool contains(const box3 &) const;
    bool intersects(const box3 &) const;
    vec3 center() const;
    double volume() const;

    static box3 init(const vec3 &, const vec3 &);
};

struct box2
{
    static const box2 empty;
    vec2 min, max;

    box2();
    box2(const vec2 &pt);
    box2(const vec2 &, const vec2 &);
    box2(const vec2 *points, size_t nPoints);

    vec2 diagonal() const;
    void inflate(const vec2 &);
    void inflate(double);
    void deflate(double);
    bool contains(const vec2 &) const;
    bool contains(const box2 &) const;
    bool intersects(const box2 &) const;
    vec2 center() const;

    static box2 init(const vec2 &, const vec2 &);
};

struct indexPair
{
    size_t p, q;

    bool operator==(const indexPair &) const;
    bool operator!=(const indexPair &) const;

    indexPair(size_t i, size_t j);
    indexPair();

    void set(size_t, size_t);
    size_t hash() const;
    void unset(size_t);
    bool add(size_t);
    bool contains(size_t) const;
};

struct indexPaidHash
{
    size_t operator()(const indexPair &) const noexcept;
};

struct customSizeTHash
{
    size_t operator()(const size_t &) const noexcept;
};

namespace utils
{
    template <typename vtype>
    void barycentricCoords(vtype const (&tri)[3], const vtype &pt, double (&coords)[3])
    {
        vtype v0 = tri[1] - tri[0], v1 = tri[2] - tri[0], v2 = pt - tri[0];
        double
            d00 = v0 * v0,
            d01 = v0 * v1,
            d11 = v1 * v1,
            d20 = v2 * v0,
            d21 = v2 * v1;
        double denom = d00 * d11 - d01 * d01;
        coords[1] = denom == 0 ? DBL_MAX_VAL : (d11 * d20 - d01 * d21) / denom;
        coords[2] = denom == 0 ? DBL_MAX_VAL : (d00 * d21 - d01 * d20) / denom;
        coords[0] = denom == 0 ? DBL_MAX_VAL : 1.0 - coords[1] - coords[2];
    };

    bool barycentricWithinBounds(double const (&coords)[3]);

    vec3 barycentricEvaluate(double const (&coords)[3], vec3 const (&pts)[3]);
}
