#pragma once
#include <vector>
#include <fstream>
#include <set>
#include "entity.h"
#include "geometry.h"
#include "wad3handler.h"


namespace M2PExport
{
	constexpr unsigned int c_VEC_GROUP_PRECISION = 1;
	static inline const char* c_NOTE_KEY{ "_note" };
	static inline const char* c_NOTE_VALUE{ "Modified by Map2Prop" };

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
		M2PGeo::Bounds bounds;
		M2PGeo::Bounds clip;
		std::string qcFlags = "";
		std::string outname;
		std::string subdir;
		std::string parent;
		std::string targetname;
		std::vector<std::string> submodels;
		std::vector<M2PGeo::Triangle> triangles;
		std::vector<M2PGeo::Bounds> alwaysSmooth;
		std::vector<M2PGeo::Bounds> neverSmooth;
		std::set<std::string> maskedTextures;
	};

	std::vector<ModelData> prepareModels(M2PEntity::BaseReader& reader, const std::string& _filename = "");
	int processModels(std::vector<ModelData>& models, bool missingTextures);
	void rewriteMap(std::vector<std::unique_ptr<M2PEntity::Entity>>& entities);
}
