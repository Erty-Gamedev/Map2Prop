#pragma once
#include <vector>
#include <fstream>
#include "entity.h"
#include "geometry.h"
#include "wad3handler.h"


namespace M2PExport
{
	enum Spawnflags
	{
		DISABLE = 1,
		IS_SUBMODEL = 2,
		RENAME_CHROME = 4,
	};

	struct ModelData
	{
		FP smoothing = 0.0, scale = 1.0, rotation = 270.0;
		bool renameChrome = false;
		M2PGeo::Vector3 offset;
		M2PGeo::Vector3 bounds[2];
		M2PGeo::Vector3 clip[2];
		std::string qcFlags = "";
		std::string outname;
		std::string subdir;
		std::vector<M2PGeo::Triangle> triangles;
		std::vector<std::array<M2PGeo::Vector3, 2>> alwaysSmooth;
		std::vector<std::array<M2PGeo::Vector3, 2>> neverSmooth;
		std::vector<std::string> maskedTextures;
	};

	class Smd
	{
	public:
		~Smd() { if (m_file.is_open()) { m_file.close(); } };
		bool writeSmd(const ModelData& model);
	private:
		std::ofstream m_file;
	};

	class Qc
	{
	public:
		~Qc() { if (m_file.is_open()) { m_file.close(); } };
		bool writeQc(const ModelData& model);
	private:
		std::ofstream m_file;
	};


	std::vector<ModelData> prepareModels(std::vector<M2PEntity::Entity>& entities, const M2PWad3::Wad3Handler& wadHandler);
	int processModels(const std::vector<ModelData>& models, bool missingTextures);
	void writeEntitiesToMap(const std::vector<M2PEntity::Entity>& entities);
}
