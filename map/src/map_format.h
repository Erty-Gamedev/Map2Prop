#pragma once

#include <fstream>
#include <filesystem>
#include <vector>
#include <array>
#include <map>
#include "geometry.h"
#include "wad3.h"
#include "entity.h"


using namespace M2PGeo;
using namespace M2PEntity;

namespace M2PMap
{
	

	class MapReader
	{
	public:
		MapReader(const std::filesystem::path&, const std::filesystem::path&);
		~MapReader();
		bool hasMissingTextures() const;
	private:
		bool m_missingTextures = false;
		std::filesystem::path m_filepath;
		std::filesystem::path m_outputDir;
		std::ifstream m_file;
		std::vector<Entity> m_entities;
		M2PWad3::Wad3Handler m_wadHandler;

		void parse();
		Entity readEntity();
		Brush readBrush();
	};
}
