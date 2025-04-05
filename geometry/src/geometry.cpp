#include "geometry.h"

using namespace M2PGeo;

Vector3D Vector3D::zero()
{
	return { 0.0f, 0.0f, 0.0f };
}
float Vector3D::magnitude() const
{
	return sqrt((x * x) + (y * y) + (z * z));
}
float Vector3D::dot(Vector3D b) const
{
	return x * b.x + y * b.y + z * b.z;
}
Vector3D Vector3D::normalized() const
{
	float mag = magnitude();
	return { x / mag, y / mag, z / mag };
}
Vector3D Vector3D::cross(Vector3D b)
{
	return {
		y * b.z - z * b.y,
		z * b.x - x * b.z,
		x * b.y - y * b.x
	};
}
Vector3D M2PGeo::Vector3D::operator+()
{
	return { x, y, z };
}
Vector3D M2PGeo::Vector3D::operator+(Vector3D& other)
{
	return { x + other.x, y + other.y, z + other.z };
}
Vector3D M2PGeo::Vector3D::operator-()
{
	return { -x, -y, -z };
}
Vector3D M2PGeo::Vector3D::operator-(Vector3D& other)
{
	return { x - other.x, y - other.y, z - other.z };
}
Vector3D M2PGeo::Vector3D::operator*(Vector3D& other)
{
	return { x * other.x, y * other.y, z * other.z };
}
Vector3D M2PGeo::Vector3D::operator*(float& other)
{
	return { x * other, y * other, z * other };
}
Vector3D M2PGeo::Vector3D::operator/(Vector3D& other)
{
	return { x / other.x, y / other.y, z / other.z };
}
Vector3D M2PGeo::Vector3D::operator/(float& other)
{
	return { x / other, y / other, z / other };
}
std::ostream& operator<<(std::ostream& os, const Vector3D v)
{
	os << "Vector3D(" << v.x << ", " << v.y << ", " << v.z << ")";
	return os;
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
	return segmentsCross(cc).normalized();
}
