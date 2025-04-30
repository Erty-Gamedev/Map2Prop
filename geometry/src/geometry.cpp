#include <format>
#include "geometry.h"


using namespace M2PGeo;


Vector2 Vector2::zero()
{
	return Vector2(0.0f, 0.0f);
}
float Vector2::magnitude() const
{
	return sqrt((x * x) + (y * y));
}
float Vector2::dot(const Vector2& other) const
{
	return x * other.x + y * other.y;
}
Vector2 Vector2::normalised() const
{
	float mag = magnitude();
	return { x / mag, y / mag };
}


Vector3 Vector3::zero()
{
	return Vector3(0.0f, 0.0f, 0.0f);
}
float Vector3::magnitude() const
{
	return sqrt((x * x) + (y * y) + (z * z));
}
float Vector3::dot(const Vector3& other) const
{
	return x * other.x + y * other.y + z * other.z;
}
Vector3 Vector3::cross(const Vector3& other) const
{
	return Vector3(
		y * other.z - z * other.y,
		z * other.x - x * other.z,
		x * other.y - y * other.x
	);
}
Vector3 Vector3::normalised() const
{
	float mag = magnitude();
	return Vector3(x / mag, y / mag, z / mag);
}
Vector3 Vector3::operator+() const
{
	return Vector3(x, y, z);
}
Vector3 Vector3::operator+(const Vector3& other) const
{
	return Vector3(x + other.x, y + other.y, z + other.z);
}
Vector3& M2PGeo::Vector3::operator+=(const Vector3& other)
{
	x += other.x;    y += other.y;    z += other.z;
	return *this;
}
Vector3 Vector3::operator-() const
{
	return Vector3(-x, -y, -z);
}
Vector3 Vector3::operator-(const Vector3& other) const
{
	return Vector3(x - other.x, y - other.y, z - other.z);
}
Vector3& M2PGeo::Vector3::operator-=(const Vector3& other)
{
	x -= other.x;    y -= other.y;    z -= other.z;
	return *this;
}
Vector3 Vector3::operator*(const Vector3& other) const
{
	return Vector3(x * other.x, y * other.y, z * other.z);
}
Vector3 Vector3::operator*(const float other) const
{
	return Vector3(x * other, y * other, z * other);
}
Vector3 Vector3::operator/(const Vector3& other) const
{
	return Vector3(x / other.x, y / other.y, z / other.z);
}
Vector3 Vector3::operator/(const float other) const
{
	return Vector3(x / other, y / other, z / other);
}
Vector3 Vector3::operator/(const int other) const
{
	return Vector3(x / other, y / other, z / other);
}
bool Vector3::operator==(const Vector3& other) const
{
	return abs(x - other.x) < c_EPSILON
		&& abs(y - other.y) < c_EPSILON
		&& abs(z - other.z) < c_EPSILON;
}
bool Vector3::operator!=(const Vector3& other) const
{
	return !(*this == other);
}

Vector3 M2PGeo::operator*(const float& lhs, const Vector3& rhs)
{
	return Vector3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}
std::ostream& M2PGeo::operator<<(std::ostream& os, const Vector3& v)
{
	os << std::format("Vector3D({:.2f}, {:.2f}, {:.2f})", v.x, v.y, v.z);
	return os;
}


Vector2 M2PGeo::Texture::uvForPoint(const Vector3& point) const
{
	return {
		(point.dot(rightaxis) / width) / scalex + shiftx / width,
		(point.dot(downaxis) / height) / scaley + shifty / height
	};
}


HessianPlane::HessianPlane(Vector3 normal, float distance)
{
	m_normal = normal;
	m_distance = distance;
}
HessianPlane::HessianPlane(const Vector3 planePoints[3])
{
	Vector3 reversed[3]{};
	for (int i = 0; i < 3; ++i)
	{
		reversed[2 - i] = planePoints[i];
	}
	m_normal = planeNormal(reversed);
	m_distance = m_normal.dot(reversed[0]);
}
Vector3 HessianPlane::normal() const { return m_normal; }
float HessianPlane::distance() const { return m_distance; }
float HessianPlane::distanceToPoint(Vector3 point) const
{
	return m_normal.dot(point - (m_normal * m_distance));
}
PointRelation HessianPlane::pointRelation(const Vector3& point) const
{
	float m_distance = distanceToPoint(point);
	if (abs(m_distance) < M2PGeo::c_EPSILON)
		return PointRelation::ON_PLANE;
	return m_distance > 0 ? PointRelation::INFRONT : PointRelation::BEHIND;
}


Plane::Plane(const Vector3 planePoints[3], const Texture& texture)
{
	for (int i = 0; i < 3; ++i)
	{
		m_planePoints[2 - i] = planePoints[i];
	}
	m_normal = planeNormal(m_planePoints);
	m_distance = m_normal.dot(m_planePoints[0]);

	m_texture = texture;
}
Texture Plane::texture() const { return m_texture; }


Vector3 M2PGeo::segmentsCross(const Vector3 planePoints[3])
{
	return (planePoints[2] - planePoints[1]).cross(planePoints[0] - planePoints[1]);
}

Vector3 M2PGeo::planeNormal(const Vector3 planePoints[3])
{
	return (planePoints[2] - planePoints[1]).cross(planePoints[0] - planePoints[1]).normalised();
}

Vector3 M2PGeo::sumVectors(const std::vector<Vector3>& vectors)
{
	Vector3 sum = Vector3::zero();
	for (const Vector3& vector : vectors)
	{
		sum += vector;
	}
	return sum;
}

Vector3 M2PGeo::geometricCenter(const std::vector<Vector3>& vectors)
{
	return sumVectors(vectors) / static_cast<int>(vectors.size());
}

void M2PGeo::sortVectors(std::vector<Vector3> &vectors, const Vector3& normal)
{
	size_t numVectors = vectors.size();
	Vector3 center = geometricCenter(vectors);
	std::vector<Vector3> rest;
	rest.reserve(numVectors - 1);
	rest.insert(rest.begin(), vectors.begin() + 1, vectors.end());
	vectors.erase(vectors.begin() + 1, vectors.end());

	Vector3 currentVect, vectOther;
	HessianPlane plane;
	float dotNormal, angleSmallest;
	size_t indexSmallest, numRest;
	while (vectors.size() < numVectors)
	{
		angleSmallest = -1.0f;
		indexSmallest = -1;

		currentVect = vectors.back();
		Vector3 planePoints[3]{currentVect, center, center + normal};
		plane = HessianPlane(planePoints);

		numRest = rest.size();
		for (int i = 0; i < numRest; ++i)
		{
			vectOther = rest[i];
			dotNormal = (currentVect - center).normalised().dot((vectOther - center).normalised());

			if (plane.pointRelation(vectOther) == PointRelation::INFRONT && dotNormal > angleSmallest)
			{
				angleSmallest = dotNormal;
				indexSmallest = i;
			}
		}
		vectors.push_back(rest[indexSmallest]);
		rest.erase(rest.begin() + indexSmallest);
	}

	Vector3 sortedPlanePoints[3]{ vectors[0], vectors[1], vectors[2] };
	Vector3 sortedNormal = planeNormal(sortedPlanePoints);
	if (normal.dot(sortedNormal) < 0.0f)
	{
		std::reverse(vectors.begin(), vectors.end());
	}
}
