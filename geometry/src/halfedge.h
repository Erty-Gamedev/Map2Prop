#pragma once

#include <vector>
#include <array>
#include <set>
#include <memory>
#include "geometry.h"


namespace M2PHalfEdge
{
	class Coord;
	struct Edge;
	struct Face;


	class Coord : public M2PGeo::Vector3
	{
	public:
		unsigned int index;
		Edge* edge = nullptr;

		Coord(unsigned int _index, const M2PGeo::Vertex vertex) : index(_index), M2PGeo::Vector3(vertex.coord()) {}

		M2PGeo::Vector3 coord() const;
	};

	struct Vertex
	{
		Coord* position = nullptr;
		M2PGeo::Vector3 normal = M2PGeo::Vector3::zero();
		M2PGeo::Vector2 uv = M2PGeo::Vector2::zero();

		Vertex() = default;
		Vertex(Coord* coord, const M2PGeo::Vector3& _normal, const M2PGeo::Vector2& _uv);
	};

	struct Edge
	{
		unsigned int index;
		Coord* origin = nullptr;
		Face* face = nullptr;
		Edge* twin = nullptr;
		Edge* next = nullptr;
		Edge* prev = nullptr;
		bool sharp{ true };
		std::set<unsigned int> faceIndices;

		Edge(
			unsigned int _index,
			Coord* _origin,
			Face* _face
		) : index(_index), origin(_origin), face(_face) {};

		bool operator==(const Edge& rhs) const;
	};

	struct Face
	{
		unsigned int index;
		M2PGeo::Vector3 normal;
		std::string textureName;
		std::array<Vertex, 3> vertices{};
		bool flipped{ false };

		Face(
			unsigned int _index,
			const M2PGeo::Vector3 _normal,
			const std::string& _textureName,
			bool _flipped = false
		) : index(_index), normal(_normal), textureName(_textureName), flipped(_flipped) {}


		M2PGeo::Vector3 fullNormal() const;

		bool operator<(const Face& rhs) const;
		bool operator==(const Face& rhs) const;
	};

	struct SmoothFan
	{
		unsigned int coordIndex;
		M2PGeo::Vector3 accumulatedNormal = M2PGeo::Vector3::zero();
		std::vector<Face*> faces;
		std::vector<M2PGeo::Vector3> normals;

		void addFace(Face* face);
		void applySmooth() const;
	};

	class Mesh
	{
	public:
		std::vector<std::unique_ptr<Coord>> coords;
		std::vector<std::unique_ptr<Edge>> edges;
		std::vector<std::unique_ptr<Face>> faces;

		Mesh() = default;
		Mesh(Mesh& other) = delete;
		~Mesh() = default;


		Coord* addVertex(const M2PGeo::Vertex _vertex);
		Edge* addEdge(Coord* origin, const Coord* end, Face* face);

		void findTwins(Edge* edge);

		void addTriangle(
			const M2PGeo::Triangle& triangle,
			const M2PGeo::Texture& texture,
			bool flipped = false
		);

		void markSmoothEdges(
			FP smoothing,
			const std::vector<M2PGeo::Bounds>& alwaysSmooth,
			const std::vector<M2PGeo::Bounds>& neverSmooth
		);
		std::vector<SmoothFan> getSmoothFansByVertex(const Coord& vertex);
	};
}
