#include <format>
#include "geometry.h"

using namespace M2PGeo;


Vector2D Vector2D::zero()
{
	return { 0.0f, 0.0f };
}
float Vector2D::magnitude() const
{
	return sqrt((x * x) + (y * y));
}
float Vector2D::dot(const Vector2D& other) const
{
	return x * other.x + y * other.y;
}
Vector2D Vector2D::normalised() const
{
	float mag = magnitude();
	return { x / mag, y / mag };
}

Vector3D Vector3D::zero()
{
	return { 0.0f, 0.0f, 0.0f };
}
float Vector3D::magnitude() const
{
	return sqrt((x * x) + (y * y) + (z * z));
}
float Vector3D::dot(const Vector3D& other) const
{
	return x * other.x + y * other.y + z * other.z;
}
Vector3D Vector3D::cross(const Vector3D& other)
{
	return
	{
		y * other.z - z * other.y,
		z * other.x - x * other.z,
		x * other.y - y * other.x
	};
}
Vector3D Vector3D::normalised() const
{
	float mag = magnitude();
	return { x / mag, y / mag, z / mag };
}
Vector3D M2PGeo::Vector3D::operator+()
{
	return { x, y, z };
}
Vector3D M2PGeo::Vector3D::operator+(const Vector3D& other)
{
	return { x + other.x, y + other.y, z + other.z };
}
Vector3D M2PGeo::Vector3D::operator-()
{
	return { -x, -y, -z };
}
Vector3D M2PGeo::Vector3D::operator-(const Vector3D& other)
{
	return { x - other.x, y - other.y, z - other.z };
}
Vector3D M2PGeo::Vector3D::operator*(const Vector3D& other)
{
	return { x * other.x, y * other.y, z * other.z };
}
Vector3D M2PGeo::Vector3D::operator*(const float& other)
{
	return { x * other, y * other, z * other };
}
Vector3D M2PGeo::Vector3D::operator/(const Vector3D& other)
{
	return { x / other.x, y / other.y, z / other.z };
}
Vector3D M2PGeo::Vector3D::operator/(const float& other)
{
	return { x / other, y / other, z / other };
}
std::ostream& operator<<(std::ostream& os, const Vector3D& v)
{
	os << std::format("Vector3D({:.2f}, {:.2f}, {:.2f})", v.x, v.y, v.z);
	return os;
}

float HessianPlane::distanceToPoint(Vector3D point)
{
	return normal.dot(point - (normal * distance));
}
PointRelation HessianPlane::pointRelation(const Vector3D& point)
{
	float distance = distanceToPoint(point);
	if (abs(distance) < M2PGeo::EPSILON)
		return PointRelation::ON_PLANE;
	return distance > 0 ? PointRelation::INFRONT : PointRelation::BEHIND;
}


Vector3D M2PGeo::segmentsCross(Vector3D planePoints[3])
{
	return (planePoints[2] - planePoints[1]).cross(planePoints[0] - planePoints[1]);
}

Vector3D Polygon::normal() {
	Vector3D cc[3]{};
	for (int i = 0; i < 3; i++)
	{
		cc[i] = vertices[i].coord;
	}
	return segmentsCross(cc).normalised();
}
