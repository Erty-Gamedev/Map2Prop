#pragma once
#include <vector>
#include <fstream>
#include <cmath>
#include <set>
#include "entity.h"
#include "geometry.h"
#include "wad3handler.h"
#include "halfedge.h"


namespace M2PExport
{
	static inline const char* c_NOTE_KEY{ "_note" };
	static inline const char* c_NOTE_VALUE{ "Modified by Map2Prop" };
	static inline const FP c_SIN45 = static_cast<FP>(sin(std::numbers::pi / 4));

	enum Spawnflags
	{
		DISABLE = 1,
		IS_SUBMODEL = 2,
		RENAME_CHROME = 4,
	};

	enum ClipGenType
	{
		BOX = 1,
		CYLINDER = 2,
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
		std::vector<M2PGeo::Bounds> alwaysSmooth;
		std::vector<M2PGeo::Bounds> neverSmooth;
		std::set<std::string> maskedTextures;
		M2PHalfEdge::Mesh mesh;

		ModelData() = default;
		ModelData(ModelData& other) = delete;
		~ModelData() = default;

		void applyOffset()
		{
			for (auto& coord : mesh.coords)
			{
				coord->x -= offset.x;
				coord->y -= offset.y;
				coord->z -= offset.z;
			}
		}
	};

	std::unordered_map<std::string, ModelData> prepareModels(M2PEntity::BaseReader& reader, const std::string& _filename = "");
	int processModels(std::unordered_map<std::string, ModelData>& models, bool missingTextures, std::vector<std::filesystem::path>& successes);
	void rewriteMap(std::vector<std::unique_ptr<M2PEntity::Entity>>& entities);
}
