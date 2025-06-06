#pragma once

#include <array>
#include <fstream>
#include <filesystem>
#include "entity.h"


namespace M2POL
{
	static inline constexpr int c_MAX_NOTES{ 501 };
	static inline const char* pLibHeader = "Worldcraft Prefab Library\r\n\x1a";
	static inline constexpr float fLibVersion = 0.1f;

#pragma pack(push, 1)
	struct PrefabLibHeader
	{
		char header[28];
		float version;
		std::uint32_t dirOffset;
		std::uint32_t numEntries;
		char notes[c_MAX_NOTES];
	};
	struct PrefabHeader
	{
		std::uint32_t offset;
		std::uint32_t size;
		char name[31];
		char notes[c_MAX_NOTES];
		std::int32_t type;
	};
#pragma pack(pop)
}

namespace M2PFormat
{
	class OlReader
	{
	public:
		OlReader(const std::filesystem::path& filepath, const std::filesystem::path& outputDir);
		~OlReader();

		int process();
	private:
		std::filesystem::path m_filepath;
		std::filesystem::path m_outputDir;
		std::ifstream m_file;
		std::vector<M2POL::PrefabHeader> m_entries;

		void parse();
	};
}
