#pragma once

#include <fstream>
#include <filesystem>
#include <vector>
#include "geometry.h"
#include "entity.h"


namespace M2PFormat
{
	class MapReader : public M2PEntity::BaseReader
	{
	public:
		MapReader(const std::filesystem::path &filepath, const std::filesystem::path &outputDir);
		~MapReader();
	private:
		std::filesystem::path m_filepath;
		std::filesystem::path m_outputDir;
		std::ifstream m_file;

		void parse();
		void readEntity(M2PEntity::Entity &entity);
		void readBrush(M2PEntity::Brush &brush, std::string& line, bool &outValid);
	};


	bool intersection3Planes(
		const M2PGeo::HessianPlane &p1,
		const M2PGeo::HessianPlane &p2,
		const M2PGeo::HessianPlane &p3,
		M2PGeo::Vector3 &intersectionOut);
	void planesToFaces(const std::vector<M2PGeo::Plane> &planes, std::vector<M2PEntity::Face> &faces);
}
