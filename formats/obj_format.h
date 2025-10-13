#pragma once

#include <vector>
#include <fstream>
#include <filesystem>
#include "entity.h"
#include "geometry.h"


namespace M2POBJ
{
	static inline const std::string c_MTLLIB_PREFIX = "mtllib ";
	static inline const std::string c_MTL_PREFIX = "mtllib ";
	static inline const std::string c_MTL_MAP_PREFIX = "map_Ka ";

	static inline const std::string c_OBJECT_PREFIX = "o ";
	static inline const std::string c_GROUP_PREFIX = "g ";
	static inline const std::string c_SMOOTH_PREFIX = "s ";
	static inline const std::string c_USEMTL_PREFIX = "usemtl ";

	static inline const std::string c_VERTEX_PREFIX = "v ";
	static inline const std::string c_UV_PREFIX = "vt ";
	static inline const std::string c_NORMAL_PREFIX = "vn ";
	static inline const std::string c_FACE_PREFIX = "f ";
}

namespace M2PFormat
{
	class ObjReader : public M2PEntity::BaseReader
	{
	public:
		ObjReader(const std::filesystem::path& filepath, const std::filesystem::path& outputDir);
		~ObjReader();
	private:
		std::filesystem::path m_filepath;
		std::filesystem::path m_outputDir;
		std::ifstream m_file;
		std::vector<M2PGeo::Vector3> m_vertexCoords;
		std::vector<M2PGeo::Vector2> m_uvCoords;
		std::vector<M2PGeo::Vector3> m_normalCoords;

		void parse();

		M2PGeo::Vector3 readCoordinate(const std::string &line);

		void readEntity(std::string &line);
		void readBrush(M2PEntity::Entity &entity, std::string &line);
		void readFace(M2PEntity::Brush &brush, std::string &line);
	};
}
