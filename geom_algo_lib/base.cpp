#include "base.h"

#define deletePtr(ptr, isArray) if (isArray) {delete[] arr;} else {delete arr;}

PINVOKE int Test_GetSquare(int n)
{
	return n * n;
}

const vec3 vec3::unset = vec3(doubleMaxValue, doubleMaxValue, doubleMaxValue);
const vec3 vec3::zero = vec3(0, 0, 0);
const vec2 vec2::zero = vec2(0, 0);
const vec2 vec2::unset = vec2(doubleMaxValue, doubleMaxValue);
const box3 box3::empty = box3(vec3::unset, -vec3::unset);
const box2 box2::empty = box2(vec2::unset, -vec2::unset);

vec3::vec3(double a, double b, double c)
	: x(a), y(b), z(c) {}

vec3::vec3(const vec3& v)
	: x(v.x), y(v.y), z(v.z) {}

vec3::vec3()
	:vec3(doubleMaxValue, doubleMaxValue, doubleMaxValue){}

vec3 vec3::operator+(const vec3& v) const
{
	return vec3(x + v.x, y + v.y, z + v.z);
}

vec3 vec3::operator-(const vec3& v) const
{
	return vec3(x - v.x, y - v.y, z - v.z);
}

double vec3::operator*(const vec3& v) const
{
	return x * v.x + y * v.y + z * v.z;
}

vec3 vec3::operator^(const vec3& v) const
{
	return vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

vec3 vec3::operator*(double s) const
{
	return vec3(x * s, y * s, z * s);
}

vec3 vec3::operator/(double s) const
{
	return vec3(x / s, y / s, z / s);
}

bool vec3::operator==(const vec3& v) const
{
	return x == v.x && y == v.y && z == v.z;
}

bool vec3::operator!=(const vec3& v) const
{
	return x != v.x || y != v.y || z != v.z;
}

void vec3::operator+=(const vec3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
}

void vec3::operator-=(const vec3& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

void vec3::operator/=(double s)
{
	x /= s;
	y /= s;
	z /= s;
}

void vec3::operator*=(double s)
{
	x /= s;
	y /= s;
	z /= s;
}

vec3 vec3::operator-() const
{
	return vec3(-x, -y, -x);
}

double vec3::len_sq() const
{
	return x * x + y * y + z * z;
}

double vec3::len() const
{
	return sqrt(len_sq());
}

void vec3::copy(double* dest, size_t& pos) const
{
	dest[pos++] = x;
	dest[pos++] = y;
	dest[pos++] = z;
}

void vec3::copy(double dest[3]) const
{
	dest[0] = x;
	dest[1] = y;
	dest[2] = z;
}

bool vec3::is_zero() const
{
	return x == 0 && y == 0 && z == 0;
}

bool vec3::is_valid() const
{
	return x != vec3::unset.x && y != vec3::unset.y && z != vec3::unset.z;
}

vec3 vec3::unit() const
{
	return is_zero() ? vec3::zero : vec3(x, y, z) / len();
}

void vec3::reverse()
{
	x = -x;
	y = -y;
	z = -z;
}

void vec3::set(double xx, double yy, double zz)
{
    x = xx;
    y = yy;
    z = zz;
}

void vec3::set(const vec3& v)
{
    set(v.x, v.y, v.z);
}

vec3 vec3::sum(const std::vector<vec3>& vecs)
{
	vec3 sum = vec3::zero;
    for (const vec3& v : vecs)
    {
        sum += v;
    }
	return sum;
}

vec3 vec3::average(const std::vector<vec3>& vecs)
{
	return sum(vecs) / (double)vecs.size();
}

vec3 vec3::min_coords(const vec3& a, const vec3& b)
{
    return vec3(
        std::min(a.x, b.x),
        std::min(a.y, b.y),
        std::min(a.z, b.z)
    ); 
}

vec3 vec3::max_coords(const vec3& a, const vec3& b)
{
    return vec3(
        std::max(a.x, b.x),
        std::max(a.y, b.y),
        std::max(a.z, b.z)
    );
}

bool index_pair::operator==(const index_pair& pair) const
{
	return (p == pair.p && q == pair.q) || (p == pair.q && q == pair.p);
}

bool index_pair::operator!=(const index_pair& pair) const
{
	return (p != pair.q && p != pair.p) || (q != pair.p && q != pair.q);
}

index_pair::index_pair(size_t i, size_t j)
	: p(i), q(j) {}

index_pair::index_pair()
	: p(-1), q(-1) {}

size_t index_pair::hash() const
{
	return p + q + p * q;
}

void index_pair::unset(size_t i)
{
	if (p == i) {
		p = -1;
	}
	else if (q == i) {
		q = -1;
	}
}

bool index_pair::add(size_t i)
{
	if (p == -1) {
		p = i;
		return true;
	}
	else if (q == -1) {
		q = i;
		return true;
	}
	return false;
}

bool index_pair::contains(size_t i) const
{
	return (i != -1) && (i == p || i == q);
}

PINVOKE void ReleaseInt(int* arr, bool isArray)
{
	deletePtr(arr, isArray);
}

PINVOKE void ReleaseDouble(double* arr, bool isArray)
{
	deletePtr(arr, isArray);
}

size_t index_pair_hash::operator()(const index_pair& ip) const noexcept
{
	return ip.hash();
}

size_t custom_size_t_hash::operator()(const size_t& i) const noexcept
{
	return i;
}

box3::box3() : min(vec3::unset), max(-vec3::unset)
{
}

box3::box3(const vec3& min, const vec3& max) : box3()
{
    inflate(min);
    inflate(max);
}

vec3 box3::diagonal() const
{
    return max - min;
}

void box3::inflate(const vec3& pt)
{
    min.set(vec3::min_coords(pt, min));
    max.set(vec3::max_coords(pt, max));
}

void box3::inflate(double d)
{
    vec3 v(d, d, d);
    min -= v;
    max += v;
}

void box3::deflate(double d)
{
    inflate(-d);
}

bool box3::contains(const vec3& pt) const
{
    return !(min.x > pt.x || max.x < pt.x || min.y > pt.y || max.y < pt.y || min.z > pt.z || max.z < pt.z);
}

bool box3::contains(const box3& b) const
{
    return contains(b.min) && contains(b.max);
}

bool box3::intersects(const box3& b) const
{
    vec3 m1 = vec3::max_coords(b.min, min);
    vec3 m2 = vec3::min_coords(b.max, max);
    vec3 d = m2 - m1;
    return d.x > 0 && d.y > 0 && d.z > 0;
}

box3 box3::init(const vec3& m1, const vec3& m2)
{
    box3 b;
    b.min = m1;
    b.max = m2;
}

vec2::vec2(double x, double y) : x(x), y(y)
{
}

vec2::vec2(const vec2& v) : vec2(v.x, v.y)
{
}

vec2::vec2() : vec2(vec2::unset)
{
}

vec2 vec2::operator+(const vec2& v) const
{
    return vec2(x + v.x, y + v.y);
}

vec2 vec2::operator-(const vec2& v) const
{
    return vec2(x - v.x, y - v.y);
}

double vec2::operator*(const vec2& v) const
{
    return x * v.x + y * v.y;
}

vec2 vec2::operator*(double s) const
{
    return vec2(x * s, y * s);
}

vec2 vec2::operator/(double s) const
{
    return vec2(x / s, y / s);
}

bool vec2::operator==(const vec2& v) const
{
    return x == v.x && y == v.y;
}

bool vec2::operator!=(const vec2& v) const
{
    return v.x != x || v.y != y;
}

void vec2::operator+=(const vec2& v)
{
    x += v.x;
    y += v.y;
}

void vec2::operator-=(const vec2& v)
{
    x -= v.x;
    y -= v.y;
}

void vec2::operator/=(double s)
{
    x /= s;
    y /= s;
}

void vec2::operator*=(double s)
{
    x *= s;
    y *= s;
}

vec2 vec2::operator-() const
{
    return vec2(-x, -y);
}

double vec2::len_sq() const
{
    return x * x + y * y;
}

double vec2::len() const
{
    return sqrt(len_sq());
}

void vec2::copy(double* dest, size_t& pos) const
{
    dest[pos++] = x;
    dest[pos++] = y;
}

void vec2::copy(double dest[2]) const
{
    dest[0] = x;
    dest[1] = y;
}

bool vec2::is_zero() const
{
    return x == 0.0 && y == 0.0;
}

bool vec2::is_valid() const
{
    return *this != vec2::unset;
}

vec2 vec2::unit() const
{
    return is_zero() ? vec2::zero : vec2(x, y) / len();
}

void vec2::reverse()
{
    x = -x;
    y = -y;
}

void vec2::set(double xx, double yy)
{
    x = xx;
    y = yy;
}

void vec2::set(const vec2& v)
{
    set(v.x, v.y);
}

vec2 vec2::sum(const std::vector<vec2>& vecs)
{
    vec2 s = vec2::zero;
    for (const vec2& v : vecs)
    {
        s += v;
    }
    return s;
}

vec2 vec2::average(const std::vector<vec2>& vecs)
{
    return sum(vecs) / (double)vecs.size();
}

vec2 vec2::min_coords(const vec2& a, const vec2& b)
{
    return vec2(std::min(a.x, b.x), std::min(a.y, b.y));
}

vec2 vec2::max_coords(const vec2& a, const vec2& b)
{
    return vec2(std::max(a.x, b.x), std::max(a.y, b.y));
}

box2::box2() : min(vec2::unset), max(-vec2::unset)
{
}

box2::box2(const vec2& a, const vec2& b) : box2()
{
    inflate(a);
    inflate(b);
}

vec2 box2::diagonal() const
{
    return max - min;
}

void box2::inflate(const vec2& pt)
{
    min.set(vec2::min_coords(min, pt));
    max.set(vec2::max_coords(max, pt));
}

void box2::inflate(double d)
{
    vec2 v(d, d);
    min -= v;
    max += v;
}

void box2::deflate(double d)
{
    inflate(-d);
}

bool box2::contains(const vec2& v) const
{
    return !(min.x > v.x || max.x < v.x || min.y > v.y || max.y < v.y);
}

bool box2::contains(const box2& b) const
{
    return contains(b.min) && contains(b.max);
}

bool box2::intersects(const box2& b) const
{
    vec2 m1 = vec2::max_coords(min, b.min);
    vec2 m2 = vec2::min_coords(max, b.max);
    vec2 d = m2 - m1;
    return d.x > 0 && d.y > 0;
}

box2 box2::init(const vec2& m1, const vec2& m2)
{
    box2 b;
    b.min = m1;
    b.max = m2;
}
