#include <set>
#include <memory>
#include "halfedge.h"
#include "logging.h"


static inline Logging::Logger& logger = Logging::Logger::getLogger("halfedge");

using namespace M2PHalfEdge;


M2PGeo::Vector3 Coord::coord() const
{
	return { x, y, z };
}


bool Edge::operator==(const Edge& rhs) const
{
	return origin->coord() == rhs.origin->coord() && next->origin->coord() == rhs.next->origin->coord();
}

M2PGeo::Vector3 Face::fullNormal() const
{
	return M2PGeo::segmentsCross(
		vertices[0].position->coord(),
		vertices[1].position->coord(),
		vertices[2].position->coord()
	);
}

bool Face::operator<(const Face& rhs) const
{
	for (int i = 0; i < 3; ++i)
		if (vertices[i].position->index != rhs.vertices[i].position->index)
			return true;
	return false;
}

bool M2PHalfEdge::Face::operator==(const Face& rhs) const
{
	for (int i = 0; i < 3; ++i)
		if (vertices[i].position->index != rhs.vertices[i].position->index)
			return false;
	return flipped == rhs.flipped;
}

static bool containsVector3(std::vector<M2PGeo::Vector3> haystack, M2PGeo::Vector3 needle)
{
	for (const M2PGeo::Vector3& item : haystack)
		if (item == needle)
			return true;
	return false;
}

void SmoothFan::addFace(Face* face)
{
	faces.push_back(face);
	if (!containsVector3(normals, face->normal))
	{
		normals.push_back(face->normal);
		accumulatedNormal += face->fullNormal();
	}
}

void SmoothFan::applySmooth() const
{
	for (auto& face : faces)
	{
		if (!face)
			continue;

		for (int i = 0; i < 3; ++i)
		{
			if (face->vertices[i].position->index == coordIndex)
			{
				face->vertices[i].normal = accumulatedNormal.normalised();
				break;
			}
		}
	}
}

Coord* Mesh::addVertex(const M2PGeo::Vertex vertex)
{
	for (const auto& coord : coords)
		if (coord->coord() == vertex.coord())
			return coord.get();
	return coords.emplace_back(std::make_unique<Coord>(static_cast<unsigned int>(coords.size()), vertex)).get();
}

Edge* Mesh::addEdge(Coord* origin, const Coord* end, Face* face)
{
	Edge* edge = nullptr;
	for (const auto& other : edges)
		if (other->origin->index == origin->index && other->next->origin->index == end->index)
		{
			edge = other.get();
			break;
		}
	if (!edge)
		edge = edges.emplace_back(std::make_unique<Edge>(static_cast<unsigned int>(edges.size()), origin, face)).get();

	edge->faceIndices.insert(face->index);
	return edge;
}


void Mesh::findTwins(Edge* edge)
{
	const int origin = edge->origin->index;
	const int end = edge->next->origin->index;

	for (auto& other : edges)
	{
		if (other->origin->index == end && other->next->origin->index == origin)
		{
			edge->twin = other.get();
			other->twin = edge;
			break;
		}
	}
}

void Mesh::addTriangle(const M2PGeo::Triangle& triangle, const M2PGeo::Texture& texture, bool flipped)
{
	const auto& pFace = faces.emplace_back(std::make_unique<Face>(
		static_cast<unsigned int>(faces.size()),
		triangle.normal,
		texture.name,
		flipped
	)).get();

	Coord* v0 = addVertex(triangle.vertices[0]);
	Coord* v1 = addVertex(triangle.vertices[1]);
	Coord* v2 = addVertex(triangle.vertices[2]);
	
	pFace->vertices[0] = Vertex{v0, pFace->normal, triangle.vertices[0].uv};
	pFace->vertices[1] = Vertex{v1, pFace->normal, triangle.vertices[1].uv};
	pFace->vertices[2] = Vertex{v2, pFace->normal, triangle.vertices[2].uv};

	Edge* pE0 = addEdge(v0, v1, pFace);
	Edge* pE1 = addEdge(v1, v2, pFace);
	Edge* pE2 = addEdge(v2, v0, pFace);

	pE0->next = pE1; pE0->prev = pE2; v0->edge = pE0;
	pE1->next = pE2; pE1->prev = pE0; v1->edge = pE1;
	pE2->next = pE0; pE2->prev = pE1; v2->edge = pE2;

	findTwins(pE0);
	findTwins(pE1);
	findTwins(pE2);
}

void Mesh::markSmoothEdges(
	FP smoothing,
	const std::vector<M2PGeo::Bounds>& alwaysSmooth,
	const std::vector<M2PGeo::Bounds>& neverSmooth)
{
	FP threshold = M2PGeo::deg2rad(smoothing);
	std::set<unsigned int> visitedEdges;

	for (auto& edge : edges)
	{
		if (visitedEdges.find(edge->index) != visitedEdges.end())
			continue;

		visitedEdges.insert(edge->index);

		if (edge->faceIndices.size() != 1)
			continue;

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

		if ((!alwaysSmooth.empty()
			&& M2PGeo::pointInBounds(edge->origin->coord(), alwaysSmooth)
			&& M2PGeo::pointInBounds(edge->next->origin->coord(), alwaysSmooth))
			|| edge->face->normal.angle(edge->twin->face->normal) < threshold
			)
		{
			edge->sharp = false;
			edge->twin->sharp = false;
			continue;
		}
	}
}

static inline SmoothFan walkSmoothFan(
	const Coord& vertex, Edge* currentEdge,
	std::set<unsigned int>& visitedFaces
)
{
	SmoothFan fan{ vertex.index };

	// If both edges from this vertex are sharp, this fan only contains this face
	if (currentEdge->sharp && currentEdge->prev && currentEdge->prev->sharp)
	{
		visitedFaces.insert(currentEdge->face->index);
		fan.addFace(currentEdge->face);
		return fan;
	}

	Edge* startEdge = currentEdge;
	bool backwards{ true };

	int g = 0;
	while (true)
	{
		if (g > 100)
		{
			logger.debug("Loop detected in %s() @ L%i", __func__, __LINE__);
			break;
		}
		++g;

		if (visitedFaces.contains(currentEdge->face->index))
		{
			backwards = false;
			break;
		}

		visitedFaces.insert(currentEdge->face->index);

		if (currentEdge->face->normal.dot(startEdge->face->normal) < 0)
		{
			if (!currentEdge->twin)
				break;

			currentEdge = currentEdge->twin->next;
			continue;
		}

		fan.addFace(currentEdge->face);

		if (!currentEdge->twin || currentEdge->sharp)
			break;

		currentEdge = currentEdge->twin->next;
	}

	if (startEdge->prev == nullptr)
		throw std::runtime_error("previous was null");

	g = 0;
	Edge* prevEdge = startEdge->prev->twin;
	while (backwards && prevEdge && !prevEdge->sharp)
	{
		if (g > 100)
		{
			logger.debug("Loop detected in %s() @ L%i", __func__, __LINE__);
			break;
		}
		++g;

		if (visitedFaces.contains(prevEdge->face->index))
			break;

		visitedFaces.insert(prevEdge->face->index);

		if (prevEdge->face->normal.dot(startEdge->face->normal) < 0)
		{
			prevEdge = prevEdge->prev->twin;
			continue;
		}

		fan.addFace(prevEdge->face);

		if (!prevEdge->prev->twin || prevEdge->prev->twin->sharp)
			break;

		prevEdge = prevEdge->prev->twin;
	}

	return fan;
}

std::vector<SmoothFan> Mesh::getSmoothFansByVertex(const Coord& vertex)
{
	std::vector<SmoothFan> fans;

	std::set<unsigned int> visitedFaces;

	Edge* currentEdge = vertex.edge;
	unsigned int startEdgeIndex = currentEdge->index;

	int g = 0;
	while (true)
	{
		if (g > 100)
		{
			logger.debug("Loop detected in %s() @ L%i", __func__, __LINE__);
			break;
		}
		++g;

		if (visitedFaces.contains(currentEdge->face->index))
		{
			if (currentEdge->index == startEdgeIndex || !currentEdge->twin)
				break;

			currentEdge = currentEdge->twin->next;
			continue;
		}

		SmoothFan fan = walkSmoothFan(vertex, currentEdge, visitedFaces);
		fan.applySmooth();

		fans.push_back(fan);

		if (!currentEdge->twin)
			break;

		currentEdge = currentEdge->twin->next;
	}

	return fans;
}
