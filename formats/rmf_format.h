#pragma once

#include <fstream>
#include <filesystem>
#include <array>
#include "entity.h"
#include "geometry.h"


namespace M2PRMF
{
	static inline constexpr std::array<int, 3> c_SUPPORTED_VERSIONS{ 16, 18, 22 };

#pragma pack(push, 1)
	struct RmfHeader
	{
		float version;
		char magic[3];
	};
	struct RGB
	{
		unsigned char r, g, b;
	};
	struct RGBA
	{
		unsigned char r, g, b, a;
	};
	struct Visgroup
	{
		char name[128];
		RGBA color;
		std::int32_t id;
		bool visible;
		char padding[3];
	};
	struct MapObjectData
	{
		std::int32_t visgroupId;
		RGB color;
		std::int32_t childCount;
	};
	struct EntityData
	{
		std::int32_t _unused;
		std::int32_t spawnflags;
		std::int32_t kvCount;
	};
#pragma pack(pop)

	constexpr float c_BASE_AXIS[18][3]{
		{0,0, 1}, {1,0,0}, {0,-1,0},  // Floor
		{0,0,-1}, {1,0,0}, {0,-1,0},  // Ceiling
		{ 1,0,0}, {0,1,0}, {0,0,-1},  // West wall
		{-1,0,0}, {0,1,0}, {0,0,-1},  // East wall
		{0, 1,0}, {1,0,0}, {0,0,-1},  // South wall
		{0,-1,0}, {1,0,0}, {0,0,-1}   // North wall
	};

	class RmfBrush : public M2PEntity::Brush
	{
	public:
		std::string getRaw() const override;
	};

	class RmfEntity : public M2PEntity::Entity
	{
	public:
		std::string toString() const override;
	};

	void textureAxisFromPlane(const M2PGeo::Vector3 normal, FP xvOut[3], FP yvOut[3]);
}

namespace M2PFormat
{
	class RmfReader : public M2PEntity::BaseReader
	{
	public:
		RmfReader(const std::filesystem::path& filepath, const std::filesystem::path& outputDir, int seekTo = 0);
		~RmfReader();
	private:
		int m_version;
		std::filesystem::path m_filepath;
		std::filesystem::path m_outputDir;
		std::ifstream m_file;

		void parse();

		void readChildren(int count, M2PEntity::Entity& parent);
		void readVisgroup();
		void readGroup(M2PEntity::Entity &parent);
		void readPath();
		void readPathNode();
		void readEntity(M2PEntity::Entity &entity);
		void readBrush(M2PEntity::Brush &brush);
		M2PEntity::Face readFace();
	};
}
