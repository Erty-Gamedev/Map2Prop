#pragma once

#include <fstream>
#include <filesystem>
#include <vector>
#include "geometry.h"
#include "wad3handler.h"
#include "entity.h"


namespace M2PMap
{

	class MapReader : M2PEntity::BaseReader
	{
	public:
		MapReader(const std::filesystem::path &filepath, const std::filesystem::path &outputDir);
		~MapReader();
		bool hasMissingTextures() const override;
		std::vector<M2PEntity::Entity>& getEntities() override;
	private:
		std::filesystem::path m_filepath;
		std::filesystem::path m_outputDir;
		std::ifstream m_file;
		std::vector<M2PEntity::Entity> m_entities;
		M2PWad3::Wad3Handler m_wadHandler;

		void parse();
		M2PEntity::Entity readEntity();
		M2PEntity::Brush readBrush();
	};


	bool intersection3Planes(
		const M2PGeo::HessianPlane &p1,
		const M2PGeo::HessianPlane &p2,
		const M2PGeo::HessianPlane &p3,
		M2PGeo::Vector3 &intersectionOut);
	std::vector<M2PEntity::Face> planesToFaces(const std::vector<M2PGeo::Plane> &planes);
}
