#pragma once
#include <vector>
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
		std::shared_ptr<Edge> edge;

		Coord(unsigned int _index, const M2PGeo::Vertex vertex) : index(_index), M2PGeo::Vector3(vertex.coord()) {}

		M2PGeo::Vector3 coord();
	};

	struct Vertex
	{
		std::shared_ptr<Coord> position;
		M2PGeo::Vector3 normal;
		M2PGeo::Vector2 uv;

		Vertex(std::shared_ptr<Coord>& coord, const M2PGeo::Vector2& _uv);
	};

	struct Edge
	{
		unsigned int index;
		std::shared_ptr<Coord> origin;
		std::shared_ptr<Face> face;
		std::shared_ptr<Edge> twin;
		std::shared_ptr<Edge> next;
		std::shared_ptr<Edge> prev;
		bool sharp{ true };

		Edge() = default;
		Edge(
			unsigned int _index,
			std::shared_ptr<Coord>& _origin,
			std::shared_ptr<Face>& _face
		) : index(_index), origin(_origin), face(_face) {};

		bool operator==(const Edge& rhs) const;
	};

	struct Face
	{
		unsigned int index;
		std::shared_ptr<Edge> edge;
		M2PGeo::Vector3 normal;
		M2PGeo::Texture texture;
		std::shared_ptr<Vertex> vertices[3];
		bool flipped{ false };

		Face() = default;
		Face(
			unsigned int _index,
			const M2PGeo::Vector3 _normal,
			const M2PGeo::Texture& _texture,
			bool _flipped = false
		) : index(_index), normal(_normal), texture(_texture), flipped(_flipped) {}

		std::shared_ptr<Vertex> getVertex(Coord coord) const;
		M2PGeo::Vector3 fullNormal() const;

		bool operator<(const Face& rhs) const;
		bool operator==(const Face& rhs) const;
	};

	struct SmoothFan
	{
		std::shared_ptr<Coord> vertex;
		M2PGeo::Vector3 accumulatedNormal = M2PGeo::Vector3::zero();
		std::vector<std::shared_ptr<Face>> faces;
		std::vector<M2PGeo::Vector3> normals;

		void addFace(std::shared_ptr<Face>& face);
		void applySmooth() const;
	};

	struct Mesh
	{
		std::vector<std::shared_ptr<Coord>> vertices;
		std::vector<std::shared_ptr<Edge>> edges;
		std::vector<std::shared_ptr<Face>> faces;


		std::shared_ptr<Coord>& addVertex(const M2PGeo::Vertex _vertex);

		void findTwins(std::shared_ptr<Edge>& edge);

		void addTriangle(
			const M2PGeo::Triangle& triangle,
			const M2PGeo::Vector3 normal,
			const M2PGeo::Texture& texture,
			bool flipped = false
		);

		void markSmoothEdges(
			FP smoothing,
			const std::vector<M2PGeo::Bounds>& alwaysSmooth,
			const std::vector<M2PGeo::Bounds>& neverSmooth
		);
		std::vector<SmoothFan> getSmoothFansByVertex(std::shared_ptr<Coord>& vertex);
	};
}
