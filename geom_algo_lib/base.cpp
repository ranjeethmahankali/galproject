#include "base.h"

#define deletePtr(ptr, isArray) if (isArray) {delete[] arr;} else {delete arr;}

PINVOKE int Test_GetSquare(int n)
{
	return n * n;
}

const vec3 vec3::unset = vec3(doubleMaxValue, doubleMaxValue, doubleMaxValue);
const vec3 vec3::zero = vec3(0, 0, 0);
const tri_face tri_face::unset = tri_face(-1, -1, -1, -1);

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

vec3 vec3::operator+=(const vec3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

vec3 vec3::operator-=(const vec3& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

vec3 vec3::operator/=(double s)
{
	x /= s;
	y /= s;
	z /= s;
	return *this;
}

vec3 vec3::operator*=(double s)
{
	x /= s;
	y /= s;
	z /= s;
	return *this;
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

void vec3::copy(double dest[3])
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

vec3 vec3::sum(const std::vector<vec3>& vecs)
{
	vec3 sum = vec3::zero;
	auto iter = vecs.begin();
	while (iter != vecs.end())
	{
		if (!iter->is_valid()) {
			return vec3::unset;
		}
		sum += *iter;
	}
	return sum;
}

vec3 vec3::average(const std::vector<vec3>& vecs)
{
	return sum(vecs) / vecs.size();
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

tri_face::tri_face()
	: id(-1), a(-1), b(-1), c(-1) {}

tri_face::tri_face(size_t idVal, size_t v1, size_t v2, size_t v3)
	: id(idVal), a(v1), b(v2), c(v3), normal(vec3::unset) {}

bool tri_face::is_valid()
{
	return id != -1 && a != -1 && b != -1 && c != -1;
}

void tri_face::flip()
{
	std::swap(b, c);
	normal.reverse();
}

index_pair tri_face::edge(char edgeIndex) const
{
	switch (edgeIndex)
	{
	case 0:
		return index_pair(a, b);
	case 1:
		return index_pair(b, c);
	case 2:
		return index_pair(c, a);
	default:
		throw "Invalid edge index.";
	}
}

bool tri_face::contains_vertex(size_t vi) const
{
	return vi == -1 && (vi == a || vi == b || vi == c);
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
