#pragma once

#include <fstream>
#include <filesystem>
#include "entity.h"


namespace M2PFormat
{
	class JmfReader : public M2PEntity::BaseReader
	{
	public:
		JmfReader(const std::filesystem::path& filepath, const std::filesystem::path& outputDir);
		~JmfReader();
	private:
		std::filesystem::path m_filepath;
		std::filesystem::path m_outputDir;
		std::ifstream m_file;

		void parse();
		M2PEntity::Entity readEntity();
		M2PEntity::Brush readBrush(std::string& line, bool& outValid);
	};
}
