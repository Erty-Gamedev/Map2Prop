#include <set>
#include "halfedge.h"


using namespace M2PHalfEdge;


M2PGeo::Vector3 Coord::coord()
{
	return { x, y, z };
}


Vertex::Vertex(std::shared_ptr<Coord>& coord, const M2PGeo::Vector3& _normal, const M2PGeo::Vector2& _uv)
{
	position = coord;
	normal = _normal;
	uv = _uv;
}

bool Edge::operator==(const Edge& rhs) const
{
	return origin->coord() == rhs.origin->coord() && next->origin->coord() == rhs.next->origin->coord();
}

std::shared_ptr<Vertex> Face::getVertex(Coord coord) const
{
	for (int i = 0; i < 3; ++i)
	{
		if (vertices[i]->position->index == coord.index)
			return vertices[i];
	}
	return std::shared_ptr<Vertex>(nullptr);
}

M2PGeo::Vector3 Face::fullNormal() const
{
	auto& a = edge->origin;
	auto& b = edge->next->origin;
	auto& c = edge->next->next->origin;

	return M2PGeo::segmentsCross(a->coord(), b->coord(), c->coord());
}

bool Face::operator<(const Face& rhs) const
{
	for (int i = 0; i < 3; ++i)
		if (vertices[i]->position->index != rhs.vertices[i]->position->index)
			return true;
	return false;
}

bool M2PHalfEdge::Face::operator==(const Face& rhs) const
{
	for (int i = 0; i < 3; ++i)
		if (vertices[i]->position->index != rhs.vertices[i]->position->index)
			return false;
	return flipped == rhs.flipped;
}

void SmoothFan::addFace(std::shared_ptr<Face>& face)
{
	faces.push_back(face);
	auto faceNormal = face->fullNormal();
	for (const M2PGeo::Vector3& normal : normals)
		if (normal == face->normal)
			return;
	normals.push_back(face->normal);
	accumulatedNormal += face->fullNormal();
}

void SmoothFan::applySmooth() const
{
	M2PGeo::Vector3 average = accumulatedNormal;

	for (auto& face : faces)
	{
		for (int i = 0; i < 3; ++i)
		{
			if (face->vertices[i]->position->index == vertex->index)
			{
				face->vertices[i]->normal = accumulatedNormal.normalised();
				break;
			}
		}
	}
}

std::shared_ptr<Coord>& Mesh::addVertex(const M2PGeo::Vertex _vertex)
{
	for (std::shared_ptr<Coord>& vertex : vertices)
	{
		if (_vertex.coord().distance(*vertex) < M2PGeo::c_EPSILON_MERGE)
			return vertex;
	}
	vertices.push_back(std::make_shared<Coord>(static_cast<unsigned int>(vertices.size()), _vertex));
	return vertices.back();
}

void Mesh::findTwins(std::shared_ptr<Edge>& edge)
{
	const std::shared_ptr<Coord>& origin = edge->origin;
	const std::shared_ptr<Coord>& end = edge->next->origin;

	for (std::shared_ptr<Edge>& other : edges)
	{
		if (other->origin->index == end->index && other->next->origin->index == origin->index)
		{
			edge->twin = other;
			other->twin = edge;
			break;
		}
	}
}

void Mesh::addTriangle(const M2PGeo::Triangle& triangle, const M2PGeo::Vector3 normal, const M2PGeo::Texture& texture, bool flipped)
{
	faces.push_back(std::make_shared<Face>(static_cast<unsigned int>(faces.size()), normal, texture, flipped));
	std::shared_ptr<Face> face = faces.back();

	std::shared_ptr<Coord> v0 = addVertex(triangle.vertices[0]);
	std::shared_ptr<Coord> v1 = addVertex(triangle.vertices[1]);
	std::shared_ptr<Coord> v2 = addVertex(triangle.vertices[2]);

	face->vertices[0] = std::make_shared<Vertex>(v0, face->normal, triangle.vertices[0].uv);
	face->vertices[1] = std::make_shared<Vertex>(v1, face->normal, triangle.vertices[1].uv);
	face->vertices[2] = std::make_shared<Vertex>(v2, face->normal, triangle.vertices[2].uv);

	edges.push_back(std::make_shared<Edge>(static_cast<unsigned int>(edges.size()), v0, face));
	std::shared_ptr<Edge> e0 = edges.back();
	edges.push_back(std::make_shared<Edge>(static_cast<unsigned int>(edges.size()), v1, face));
	std::shared_ptr<Edge> e1 = edges.back();
	edges.push_back(std::make_shared<Edge>(static_cast<unsigned int>(edges.size()), v2, face));
	std::shared_ptr<Edge> e2 = edges.back();

	face->edge = e0;

	e0->next = e1; e0->prev = e2; v0->edge = e0;
	e1->next = e2; e1->prev = e0; v1->edge = e1;
	e2->next = e0; e2->prev = e1; v2->edge = e2;

	findTwins(e0);
	findTwins(e1);
	findTwins(e2);
}

void Mesh::markSmoothEdges(
	FP smoothing,
	const std::vector<M2PGeo::Bounds>& alwaysSmooth,
	const std::vector<M2PGeo::Bounds>& neverSmooth)
{
	FP threshold = M2PGeo::deg2rad(smoothing);
	std::set<unsigned int> visitedEdges;

	for (std::shared_ptr<Edge>& edge : edges)
	{
		if (visitedEdges.find(edge->index) != visitedEdges.end())
			continue;

		visitedEdges.insert(edge->index);

		if (!edge->face || !edge->twin || !edge->twin->face)
			continue;

		visitedEdges.insert(edge->twin->index);

		if (!neverSmooth.empty()
			&& M2PGeo::pointInBounds(edge->origin->coord(), neverSmooth)
			&& M2PGeo::pointInBounds(edge->next->origin->coord(), neverSmooth)
			)
		{
			continue;
		}

		if (!alwaysSmooth.empty()
			&& M2PGeo::pointInBounds(edge->origin->coord(), alwaysSmooth)
			&& M2PGeo::pointInBounds(edge->next->origin->coord(), alwaysSmooth)
			)
		{
			edge->sharp = false;
			edge->twin->sharp = false;
			continue;
		}

		if (edge->face->normal.angle(edge->twin->face->normal) < threshold)
		{
			edge->sharp = false;
			edge->twin->sharp = false;
		}
	}
}

static inline SmoothFan walkSmoothFan(
	std::shared_ptr<Coord>& vertex, std::shared_ptr<Edge>& currentEdge,
	std::set<unsigned int>& visitedFaces
)
{
	SmoothFan fan{ vertex };

	// If both edges from this vertex are sharp, this fan only contains this face
	if (currentEdge->sharp && currentEdge->prev->sharp)
	{
		visitedFaces.insert(currentEdge->face->index);
		fan.addFace(currentEdge->face);
		return fan;
	}

	std::shared_ptr<Edge> startEdge = currentEdge;
	bool backwards{ true };

	int g = 0;
	while (true)
	{
		if (g > 100)
			throw std::runtime_error("loop detected");

		if (visitedFaces.find(currentEdge->face->index) != visitedFaces.end())
		{
			backwards = false;
			break;
		}

		visitedFaces.insert(currentEdge->face->index);
		fan.addFace(currentEdge->face);

		if (!currentEdge->twin || currentEdge->sharp)
			break;

		currentEdge = currentEdge->twin->next;
		++g;
	}

	g = 0;
	std::shared_ptr<Edge>& prevEdge = startEdge->prev->twin;
	while (backwards && prevEdge && !prevEdge->sharp)
	{
		if (g > 100)
			throw std::runtime_error("loop detected");

		if (visitedFaces.find(prevEdge->face->index) != visitedFaces.end())
			break;

		visitedFaces.insert(prevEdge->face->index);
		fan.addFace(prevEdge->face);

		if (!prevEdge->prev->twin || prevEdge->prev->twin->sharp)
			break;

		prevEdge = prevEdge->prev->twin;
		++g;
	}

	return fan;
}

std::vector<SmoothFan> Mesh::getSmoothFansByVertex(std::shared_ptr<Coord>& vertex)
{
	std::vector<SmoothFan> fans;

	std::set<unsigned int> visitedFaces;

	std::shared_ptr<Edge> currentEdge = vertex->edge;

	while (true)
	{
		if (visitedFaces.find(currentEdge->face->index) != visitedFaces.end())
			break;

		SmoothFan fan = walkSmoothFan(vertex, currentEdge, visitedFaces);
		fan.applySmooth();

		fans.push_back(fan);

		if (!currentEdge->twin)
			break;

		currentEdge = currentEdge->twin->next;
	}

	return fans;
}
