#include <format>
#include "geometry.h"


using namespace M2PGeo;


Vector2 Vector2::zero()
{
	return Vector2(0.0f, 0.0f);
}
FP Vector2::magnitude() const
{
	return sqrt((x * x) + (y * y));
}
FP Vector2::dot(const Vector2 &other) const
{
	return x * other.x + y * other.y;
}
FP M2PGeo::Vector2::cross(const Vector2 &other) const
{
	return x * other.y - y * other.x;
}
Vector2 Vector2::normalised() const
{
	FP mag = magnitude();
	return { x / mag, y / mag };
}
bool Vector2::operator==(const Vector2& other) const
{
	return abs(x - other.x) < c_EPSILON && abs(y - other.y) < c_EPSILON;
}
bool Vector2::operator!=(const Vector2& other) const { return !(*this == other); }


Vector3 Vector3::zero()
{
	return Vector3(0.0f, 0.0f, 0.0f);
}
FP Vector3::magnitude() const
{
	return sqrt((x * x) + (y * y) + (z * z));
}
FP Vector3::dot(const Vector3& other) const
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
	FP mag = magnitude();
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
Vector3 Vector3::operator*(const FP other) const
{
	return Vector3(x * other, y * other, z * other);
}
Vector3 Vector3::operator/(const Vector3& other) const
{
	return Vector3(x / other.x, y / other.y, z / other.z);
}
Vector3 Vector3::operator/(const FP other) const
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
bool Vector3::operator!=(const Vector3& other) const { return !(*this == other); }

Vector3 M2PGeo::operator*(const FP& lhs, const Vector3& rhs)
{
	return Vector3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}
std::ostream& M2PGeo::operator<<(std::ostream& os, const Vector3& v)
{
	os << std::format("Vector3D({:.3f}, {:.3f}, {:.3f})", v.x, v.y, v.z);
	return os;
}
bool M2PGeo::operator==(const Vector3& lhs, const Vertex& rhs)
{
	return lhs == rhs.coord();
}
bool M2PGeo::operator==(const Vertex& lhs, const Vector3& rhs)
{
	return lhs.coord() == rhs;
}


bool Vertex::operator==(const Vertex& other) const
{
	// TODO: Do we need to check if both are flipped?
	return uv == other.uv && normal == other.normal && (*this).coord() == other.coord();
}
bool Vertex::operator!=(const Vertex& other) const { return !(*this == other); }



Vector2 M2PGeo::Texture::uvForPoint(const Vector3& point) const
{
	return {
		(point.dot(rightaxis) / width) / scalex + shiftx / width,
		-((point.dot(downaxis) / height) / scaley + shifty / height)
	};
}


HessianPlane::HessianPlane(Vector3 normal, FP distance)
{
	m_normal = normal;
	m_distance = distance;
}
HessianPlane::HessianPlane(const Vector3 planePoints[3])
{
	m_normal = planeNormal(planePoints);
	m_distance = m_normal.dot(planePoints[0]);
}
Vector3 HessianPlane::normal() const { return m_normal; }
FP HessianPlane::distance() const { return m_distance; }
FP HessianPlane::distanceToPoint(Vector3 point) const
{
	return m_normal.dot(point - (m_normal * m_distance));
}
PointRelation HessianPlane::pointRelation(const Vector3& point) const
{
	FP m_distance = distanceToPoint(point);
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


Vector3 M2PGeo::segmentsCross(const Vector3& a, const Vector3& b, const Vector3& c)
{
	return (b - a).cross(c - a);
}
Vector3 M2PGeo::segmentsCross(const Vector3 planePoints[3])
{
	return M2PGeo::segmentsCross(planePoints[0], planePoints[1], planePoints[2]);
}

Vector3 M2PGeo::planeNormal(const Vector3 planePoints[3])
{
	return (planePoints[2] - planePoints[1]).cross(planePoints[0] - planePoints[1]).normalised();
}

Vector3 M2PGeo::sumVectors(const std::vector<Vector3> &vectors)
{
	Vector3 sum = Vector3::zero();
	for (const Vector3& vector : vectors)
	{
		sum += vector;
	}
	return sum;
}
Vector3 M2PGeo::sumVertices(const std::vector<Vertex> &vertices)
{
	Vector3 sum = Vector3::zero();
	for (const Vertex& vertex : vertices)
	{
		sum += vertex.coord();
	}
	return sum;
}

Vector3 M2PGeo::geometricCenter(const std::vector<Vector3> &vectors)
{
	return sumVectors(vectors) / static_cast<int>(vectors.size());
}
Vector3 M2PGeo::geometricCenter(const std::pair<Vector3, Vector3>& vectors)
{
	return sumVectors(std::vector{ vectors.first, vectors.second }) / 2;
}
Vector3 M2PGeo::geometricCenter(const std::vector<Vertex> &vertices)
{
	return sumVertices(vertices) / static_cast<int>(vertices.size());
}

void M2PGeo::sortVectors(std::vector<Vector3> &vectors, const Vector3 &normal)
{
	size_t numVectors = vectors.size();
	Vector3 center = geometricCenter(vectors);
	std::vector<Vector3> rest;
	rest.reserve(numVectors - 1);
	rest.insert(rest.begin(), vectors.begin() + 1, vectors.end());
	vectors.erase(vectors.begin() + 1, vectors.end());

	Vector3 currentVect, vectOther;
	HessianPlane plane;
	FP dotNormal, angleSmallest;
	int indexSmallest, numRest;
	while (vectors.size() < numVectors)
	{
		angleSmallest = -1.0f;
		indexSmallest = -1;

		currentVect = vectors.back();
		Vector3 planePoints[3]{currentVect, center, center + normal};
		plane = HessianPlane(planePoints);

		numRest = static_cast<int>(rest.size());
		for (int i = 0; i < numRest; ++i)
		{
			if (numRest == 1)
			{
				indexSmallest = 0;
				break;
			}

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

void M2PGeo::sortVertices(std::vector<Vertex> &vertices, const Vector3& normal)
{
	size_t numVectors = vertices.size();
	Vector3 center = geometricCenter(vertices);
	std::vector<Vertex> rest;
	rest.reserve(numVectors - 1);
	rest.insert(rest.begin(), vertices.begin() + 1, vertices.end());
	vertices.erase(vertices.begin() + 1, vertices.end());

	Vector3 currentVect, vectOther;
	HessianPlane plane;
	FP dotNormal, angleSmallest;
	int indexSmallest, numRest;
	while (vertices.size() < numVectors)
	{
		angleSmallest = -1.0f;
		indexSmallest = -1;

		currentVect = vertices.back().coord();
		Vector3 planePoints[3]{ currentVect, center, center + normal };
		plane = HessianPlane(planePoints);

		numRest = static_cast<int>(rest.size());
		for (int i = 0; i < numRest; ++i)
		{
			if (numRest == 1)
			{
				indexSmallest = 0;
				break;
			}

			vectOther = rest[i].coord();
			dotNormal = (currentVect - center).normalised().dot((vectOther - center).normalised());

			if (plane.pointRelation(vectOther) == PointRelation::INFRONT && dotNormal > angleSmallest)
			{
				angleSmallest = dotNormal;
				indexSmallest = i;
			}
		}
		vertices.push_back(rest[indexSmallest]);
		rest.erase(rest.begin() + indexSmallest);
	}

	Vector3 sortedPlanePoints[3]{ vertices[0], vertices[1], vertices[2] };
	Vector3 sortedNormal = planeNormal(sortedPlanePoints);
	if (normal.dot(sortedNormal) < 0.0f)
	{
		std::reverse(vertices.begin(), vertices.end());
	}
}
