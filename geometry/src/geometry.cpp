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
Vector3 Vector3::operator-() const
{
	return Vector3(-x, -y, -z);
}
Vector3 Vector3::operator-(const Vector3& other) const
{
	return Vector3(x - other.x, y - other.y, z - other.z);
}
Vector3 Vector3::operator*(const Vector3& other) const
{
	return Vector3(x * other.x, y * other.y, z * other.z);
}
Vector3 Vector3::operator*(const float& other) const
{
	return Vector3(x * other, y * other, z * other);
}
Vector3 Vector3::operator/(const Vector3& other) const
{
	return Vector3(x / other.x, y / other.y, z / other.z);
}
Vector3 Vector3::operator/(const float& other) const
{
	return Vector3(x / other, y / other, z / other);
}
bool Vector3::operator==(const Vector3& other) const
{
	return abs(x - other.x) < EPSILON
		&& abs(y - other.y) < EPSILON
		&& abs(z - other.z) < EPSILON;
}
bool Vector3::operator!=(const Vector3& other) const
{
	return !(*this == other);
}
std::ostream& operator<<(std::ostream& os, const Vector3& v)
{
	os << std::format("Vector3D({:.2f}, {:.2f}, {:.2f})", v.x, v.y, v.z);
	return os;
}


Vector3 Polygon::normal() const
{
	Vector3 cc[3]{};
	for (int i = 0; i < 3; i++)
	{
		cc[i] = vertices[i].coord;
	}
	return segmentsCross(cc).normalised();
}


float HessianPlane::distanceToPoint(Vector3 point) const
{
	return normal.dot(point - (normal * distance));
}
PointRelation HessianPlane::pointRelation(const Vector3& point) const
{
	float distance = distanceToPoint(point);
	if (abs(distance) < M2PGeo::EPSILON)
		return PointRelation::ON_PLANE;
	return distance > 0 ? PointRelation::INFRONT : PointRelation::BEHIND;
}


Vector3 M2PGeo::segmentsCross(const Vector3 planePoints[3])
{
	return (planePoints[2] - planePoints[1]).cross(planePoints[0] - planePoints[1]);
}
