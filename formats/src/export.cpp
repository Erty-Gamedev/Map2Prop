#include <fstream>
#include <filesystem>
#include <format>
#include <unordered_map>
#include "export.h"
#include "config.h"
#include "logging.h"
#include "utils.h"
#include "ear_clip.h"


static inline Logging::Logger& logger = Logging::Logger::getLogger("export");

using M2PConfig::g_config;

using namespace M2PExport;
using namespace M2PGeo;
namespace fs = std::filesystem;


static inline std::string formatGroupedVector(const Vector3& vector)
{
	return std::format("{:.{}f},{:.{}f},{:.{}f}",
		vector.x, c_VEC_GROUP_PRECISION,
		vector.y, c_VEC_GROUP_PRECISION,
		vector.z, c_VEC_GROUP_PRECISION
	);
}

static inline void mergeNearby(ModelData& model)
{
	std::unordered_map<std::string, Vertex> vertices;
	vertices.reserve(model.triangles.size() * 3);

	for (Triangle& triangle : model.triangles)
	{
		for (Vertex& vertex : triangle.vertices)
		{
			std::string grouped = formatGroupedVector(vertex.coord());

			if (vertices.contains(grouped))
			{
				vertex.x = vertices[grouped].x;
				vertex.y = vertices[grouped].y;
				vertex.z = vertices[grouped].z;
			}
			else
			{
				vertices[grouped] = vertex;
			}
		}
	}
}

static inline bool pointInBounds(Vector3 point, const std::vector<Bounds>& bounds)
{
	for (const Bounds& b : bounds)
	{
		if (b.pointInside(point))
			return true;
	}
	return false;
}

static inline void applySmooth(ModelData& model)
{
	mergeNearby(model);
	if (model.smoothing == 0)
		return;

	size_t vertCount = model.triangles.size() * 3;
	GroupedVertices vertices;
	GroupedVertices flipped;
	GroupedVertices alwaysSmooth;
	GroupedVertices alwaysSmoothFlipped;
	vertices.reserve(vertCount);
	flipped.reserve(vertCount);
	alwaysSmooth.reserve(vertCount);
	alwaysSmoothFlipped.reserve(vertCount);

	bool neverSmooth = !model.neverSmooth.empty();

	for (Triangle& triangle : model.triangles)
	{
		for (Vertex& vertex : triangle.vertices)
		{
			if (neverSmooth && pointInBounds(vertex.coord(), model.neverSmooth))
				continue;

			bool shouldAlwaysSmooth = pointInBounds(vertex.coord(), model.alwaysSmooth);

			GroupedVertices& vList = vertices;
			if (triangle.flipped)
				vList = shouldAlwaysSmooth ? alwaysSmoothFlipped : flipped;
			else
				vList = shouldAlwaysSmooth ? alwaysSmooth : vertices;

			std::string grouped = formatGroupedVector(vertex.coord());
			vList[grouped].push_back(vertex);
		}
	}

	averageNearNormals(vertices, model.smoothing);
	averageNearNormals(flipped, model.smoothing);
	averageNormals(alwaysSmooth);
	averageNormals(alwaysSmoothFlipped);
}

static inline int compileModel(const ModelData& model)
{
	fs::path currentDir = fs::absolute(fs::current_path());
	fs::current_path(fs::current_path() / g_config.outputDir);
	int returnCode = std::system((g_config.studiomdl.string() + " " + model.outname + ".qc").c_str());
	fs::current_path(currentDir);
	return returnCode;
}


bool Smd::writeSmd(const ModelData& model)
{
	fs::path filepath = g_config.outputDir / (model.outname + ".smd");
	m_file.open(filepath);
	if (!m_file.is_open() || !m_file.good())
	{
		m_file.close();
		logger.warning("Could not open file for writing: " + filepath.string());
		return false;
	}

	logger.debug("Writing " + filepath.string());

	m_file << "version 1\nnodes\n0 \"root\" -1\nend\nskeleton\ntime 0\n0 0 0 0 0 0 0\nend\ntriangles\n";

	for (const M2PGeo::Triangle &triangle : model.triangles)
	{
		m_file << triangle.textureName << ".bmp\n";
		for (const M2PGeo::Vertex &vertex : triangle.vertices)
		{
			m_file << "0\t";
			const M2PGeo::Vector3& normal = vertex.normal;
			if (M2PConfig::isObj())
			{
				m_file << std::format("{:.6f} {:.6f} {:.6f}\t", vertex.x, -vertex.z, vertex.y);
				m_file << std::format("{:.6f} {:.6f} {:.6f}\t", normal.x, -normal.z, normal.y);
				m_file << std::format("{:.6f} {:.6f}", vertex.uv.x, vertex.uv.y + 1);
			}
			else
			{
				m_file << std::format("{:.6f} {:.6f} {:.6f}\t", vertex.x, vertex.y, vertex.z);
				m_file << std::format("{:.6f} {:.6f} {:.6f}\t", normal.x, normal.y, normal.z);
				m_file << std::format("{:.6f} {:.6f}", vertex.uv.x, vertex.uv.y + 1);
			}
			m_file << "\n";
		}
	}
	m_file << "end\n";
	m_file.close();

	logger.info("Successfully written " + filepath.string());
	return true;
}

bool Qc::writeQc(const ModelData& model)
{
	fs::path filepath = g_config.outputDir / (model.outname + ".qc");
	m_file.open(filepath);
	if (!m_file.is_open() || !m_file.good())
	{
		m_file.close();
		logger.warning("Could not open file for writing: " + filepath.string());
		return false;
	}

	std::string rendermodes{ "" };
	std::string offset{ "0 0 0" };
	std::string qcFlags = !model.qcFlags.empty() ? std::format("$flags {}\n", model.qcFlags) : "";
	std::string bbox{ "" };
	std::string cbox{ "" };

	if (!model.maskedTextures.empty())
	{
		for (const std::string& masked : model.maskedTextures)
		{
			rendermodes += "$texrendermode " + masked + ".bmp masked\n";
		}
	}

	Vector3 qcOffset{ g_config.qcOffset[0], g_config.qcOffset[1], g_config.qcOffset[2] };
	if (model.offset != Vector3::zero())
	{
		offset = std::format("{:.1f} {:.1f} {:.1f}", model.offset.x, model.offset.y, model.offset.z);
	}
	else if (qcOffset != Vector3::zero())
	{
		offset = std::format("{:.1f} {:.1f} {:.1f}", qcOffset.x, qcOffset.y, qcOffset.z);
	}

	if (model.bounds != Bounds::zero())
	{
		Vector3 bmin = model.bounds.min - model.offset;
		Vector3 bmax = model.bounds.max - model.offset;
		bbox = std::format("$bbox {} {}\n", bmin, bmax);
	}

	if (model.clip != Bounds::zero())
	{
		Vector3 bmin = model.clip.min - model.offset;
		Vector3 bmax = model.clip.max - model.offset;
		cbox = std::format("$cbox {} {}\n", bmin, bmax);
	}

	logger.debug("Writing " + filepath.string());

	m_file << "/*\n Automatically generated by Erty's GoldSrc Map2Prop.\n*/\n\n";
	m_file << "$modelname " << model.subdir << model.outname << ".mdl\n";
	m_file << "$cd \".\"\n$cdtexture \".\"\n";
	m_file << "$scale " << model.scale << "\n";
	m_file << "$origin " << offset << " " << model.rotation << "\n";
	m_file << qcFlags << rendermodes << bbox << cbox << "$gamma " << g_config.qcGamma << "\n";
	m_file << "$body studio \"" << model.outname << "\"\n";
	m_file << "$sequence \"Generated_with_Erty's_Map2Prop\" \"" << model.outname << "\"\n";

	m_file.close();

	logger.info("Successfully written " + filepath.string());
	return true;
}


std::vector<ModelData> M2PExport::prepareModels(std::vector<M2PEntity::Entity>& entities, const M2PWad3::Wad3Handler& wadHandler)
{
	int n = 0;
	std::unordered_map<std::string, ModelData> modelsMap;
	std::string keyvalue;
	keyvalue.reserve(256);

	for (M2PEntity::Entity& entity : entities)
	{
		bool isWorldspawn = entity.classname == "worldspawn";
		bool isFuncM2P = entity.classname == "func_map2prop";

		if (isWorldspawn)
		{
			if (!M2PConfig::isMap())
			{
				std::string wads;
				wads.reserve(1024);
				for (int i = 0; i < wadHandler.usedWads.size(); ++i) {
					const fs::path& wadPath = wadHandler.usedWads[i];
					std::string wadStr = "/" + fs::relative(wadPath, wadPath.root_path()).string();
					std::replace(wadStr.begin(), wadStr.end(), '\\', '/');
					wads.append(wadStr);
					if (i < wadHandler.usedWads.size() - 1)
						wads.append(";");
				}
				entity.keyvalues.emplace_back("wad", wads);
			}
			entity.setKey(c_NOTE_KEY, c_NOTE_VALUE);
		}

		if (entity.brushes.empty())
			continue;

		if (g_config.mapcompile && !isFuncM2P)
			continue;

		std::string filename = g_config.inputFilepath.stem().string();
		std::string outname = !g_config.outputName.empty() ? g_config.outputName : filename;
		bool ownModel = false;
		std::string subdir = "";

		if (isFuncM2P)
		{
			if (entity.getKeyInt("spawnflags") & Spawnflags::DISABLE)
				continue;

			keyvalue = entity.getKey("parent_model");
			if (keyvalue != "")
			{
				for (const auto& brush : entity.brushes)
				{
					if (brush.faces.empty())
						continue;

					if (brush.isToolBrush(M2PEntity::ToolTexture::ORIGIN))
					{
						Vector3 ori = brush.getCenter();
						entity.setKey("origin", std::format("{:.1f} {:.1f} {:.1f}", ori.x, ori.y, ori.z));
						break;
					}
				}
				continue;
			}

			if (g_config.mapcompile || entity.getKeyInt("own_model") > 0)
			{
				ownModel = true;
				outname = std::format("{}_{}", filename, n);
				keyvalue = entity.getKey("outname");
				if (!keyvalue.empty())
				{
					outname = keyvalue;
					M2PUtils::replaceToken(outname, ".mdl", "");
					if (modelsMap.contains(outname))
					{
						outname = std::format("{}_{}", outname, n);
						++n;
					}
				}
			}

			keyvalue = entity.getKey("subdir");
			if (!keyvalue.empty())
			{
				subdir = keyvalue;
			}

			fs::path parentFolder;
			if (g_config.mapcompile && !M2PConfig::modDir().empty())
				parentFolder = M2PConfig::modDir() / "models" / g_config.outputDir / subdir;
			else
				parentFolder = g_config.outputDir / subdir;

			if (!fs::is_directory(parentFolder))
				fs::create_directories(parentFolder);

			std::string modelPath = ("models" / g_config.outputDir / subdir / (outname + ".mdl")).string();
			std::replace(modelPath.begin(), modelPath.end(), '\\', '/');
			entity.setKey("model", modelPath);
		}


		float scale = g_config.qcScale;
		float rotation = g_config.qcRotate;
		float smoothing = g_config.smoothing;
		bool chrome = g_config.renameChrome;
		std::string qcFlags = "";

		if (isWorldspawn || ownModel)
		{
			if (!(keyvalue = entity.getKey("scale")).empty())
			{
				scale = std::stof(keyvalue);
				if (scale == 0)
					scale = 1.0;
			}

			if (!(keyvalue = entity.getKey("angles")).empty())
			{
				std::vector<std::string> angles = M2PUtils::split(keyvalue, ' ');
				if (angles.size() == 1)
					rotation = fmod(rotation + std::stof(angles[0]), 360.0f);
				else if (angles.size() > 2)
					rotation = fmod(rotation + std::stof(angles[1]), 360.0f);
			}

			if (!(keyvalue = entity.getKey("smoothing")).empty())
				smoothing = std::stof(keyvalue);

			if (!(keyvalue = entity.getKey("qc_flags")).empty())
				qcFlags = keyvalue;

			chrome = entity.getKeyInt("chrome") == 1;
		}

		
		if (!modelsMap.contains(outname))
		{
			modelsMap[outname].outname = outname;
			modelsMap[outname].triangles.reserve(256);
			modelsMap[outname].scale = scale;
			modelsMap[outname].rotation = rotation;
			modelsMap[outname].smoothing = smoothing;
			modelsMap[outname].renameChrome = chrome;
			modelsMap[outname].qcFlags = qcFlags;
		}

		bool originFound = false, boundsFound = false, clipFound = false;
		for (const M2PEntity::Brush& brush : entity.brushes)
		{

			// Look for ORIGIN brushes, use first found
			if (modelsMap[outname].offset == Vector3::zero() && brush.isToolBrush(M2PEntity::ToolTexture::ORIGIN))
			{
				if (originFound)
				{
					logger.info(std::format("Multiple ORIGIN brushes found in {} near ({})", entity.classname, brush.getCenter()));
					continue;
				}
				if (isWorldspawn || ownModel)
				{
					Vector3 origin = geometricCenter(brush.getBounds());
					modelsMap[outname].offset = origin;
					entity.setKey("origin", std::format("{}", origin));
				}
				originFound = true;
				continue;
			}

			// Look for BOUNDINGBOX brushes, use first found
			if (modelsMap[outname].bounds == Bounds::zero()
				&& brush.isToolBrush(M2PEntity::ToolTexture::BOUNDINGBOX))
			{
				if (boundsFound)
				{
					logger.info(std::format("Multiple BOUNDINGBOX brushes found in {} near ({})", entity.classname, brush.getCenter()));
					continue;
				}
				if (isWorldspawn || ownModel)
				{
					modelsMap[outname].bounds = brush.getBounds();
				}
				boundsFound = true;
				continue;
			}
			
			// Look for CLIP brushes, use first found
			if (modelsMap[outname].clip == Bounds::zero()
				&& brush.isToolBrush(M2PEntity::ToolTexture::CLIP))
			{
				if (clipFound)
				{
					logger.info(std::format("Multiple CLIP brushes found in {} near ({})", entity.classname, brush.getCenter()));
					continue;
				}
				if (isWorldspawn || ownModel)
				{
					modelsMap[outname].clip = brush.getBounds();
				}
				clipFound = true;
				continue;
			}
			
			// Look for CLIPBEVEL brushes
			if (brush.isToolBrush(M2PEntity::ToolTexture::CLIPBEVEL))
			{
				if (isWorldspawn || ownModel)
				{
					modelsMap[outname].neverSmooth.push_back(brush.getBounds());
				}
				continue;
			}
			
			// Look for BEVEL brushes
			if (brush.isToolBrush(M2PEntity::ToolTexture::BEVEL))
			{
				if (isWorldspawn || ownModel)
				{
					modelsMap[outname].alwaysSmooth.push_back(brush.getBounds());
				}
				continue;
			}

			for (const M2PEntity::Face& face : brush.faces)
			{
				if (M2PWad3::Wad3Handler::isSkipTexture(face.texture.name) || M2PWad3::Wad3Handler::isToolTexture(face.texture.name))
					continue;

				if (face.texture.name.starts_with('{'))
					modelsMap[outname].maskedTextures.insert(face.texture.name);

				M2PUtils::extendVector(modelsMap[outname].triangles, earClip(face.vertices, face.normal, face.texture.name));
			}

			if (brush.hasContentWater())
			{
				std::vector<Triangle> flipped{ modelsMap[outname].triangles };
				for (Triangle& triangle : flipped)
				{
					triangle.normal = -triangle.normal;
					std::swap(triangle.vertices[0], triangle.vertices[2]);
					triangle.flipped = true;
				}
				M2PUtils::extendVector(modelsMap[outname].triangles, flipped);
			}

		}

		if (modelsMap[outname].offset == Vector3::zero()
			&& (!entity.hasKey("use_world_origin") || entity.getKeyInt("use_world_origin")))
		{
			Vector3 aabbMin = modelsMap[outname].triangles[0].vertices[0].coord();
			Vector3 aabbMax = modelsMap[outname].triangles[0].vertices[0].coord();

			for (const auto& triangle : modelsMap[outname].triangles)
			{
				for (const auto& vertex : triangle.vertices)
				{
					if (vertex.x < aabbMin.x) aabbMin.x = vertex.x;
					if (vertex.y < aabbMin.y) aabbMin.y = vertex.y;
					if (vertex.z < aabbMin.z) aabbMin.z = vertex.z;

					if (vertex.x > aabbMax.x) aabbMax.x = vertex.x;
					if (vertex.y > aabbMax.y) aabbMax.y = vertex.y;
					if (vertex.z > aabbMax.z) aabbMax.z = vertex.z;
				}
			}
			modelsMap[outname].offset = geometricCenter(std::vector{ aabbMin, aabbMax });
			modelsMap[outname].offset.z -= (aabbMax.z - aabbMin.z) / 2;
		}
	}

	std::vector<ModelData> models;
	for (const auto& kv : modelsMap)
		models.push_back(kv.second);

	return models;
}


int M2PExport::processModels(std::vector<ModelData>& models, bool missingTextures)
{
	int returnCodes = 0;

	logger.debug(std::format("Processing {} model{}", models.size(), models.size() > 1 ? "s" : ""));

	for (ModelData& model : models)
	{
		applySmooth(model);

		bool smdOk = Smd().writeSmd(model);
		bool qcOk = Qc().writeQc(model);
	}

	logger.info(std::format("Finished processing {} model{}", models.size(), models.size() > 1 ? "s" : ""));

	if (!g_config.autocompile)
		return 0;

	logger.info(std::string{ "Autocompile enabled, compiling model" } + (models.size() > 1 ? "s" : "") + "...");

	if (missingTextures)
	{
		logger.warning("Cannot compile model, model has missing textures. Check logs for more info");
		return 1;
	}
	if (g_config.studiomdl.empty())
	{
		logger.warning("Cannot compile model, StudioMDL path not specified");
		return 1;
	}
	if (!fs::exists(g_config.studiomdl))
	{
		logger.warning("Cannot compile model, StudioMDL \"" + g_config.studiomdl.string() + "\" does not exist");
		return 1;
	}

	for (const ModelData& model : models)
	{
		returnCodes += compileModel(model);
	}

	if (returnCodes > 0)
		logger.warning("Something went wrong during compilation. Check logs for more info");
	else
		logger.info(std::format("Finished compiling {} model{}", models.size(), models.size() > 1 ? "s" : ""));

	return returnCodes;
}

void M2PExport::rewriteMap(std::vector<M2PEntity::Entity>& entities)
{
	std::string stem = g_config.inputFilepath.stem().string();

	fs::path filepath;
	if (M2PConfig::isMap())
	{
		// TODO: Only make a M2P copy if we're working on an unmodified MAP
		fs::path copyPath = g_config.inputDir / (stem + ".m2p");
		std::filesystem::copy_file(g_config.inputFilepath, copyPath, fs::copy_options::overwrite_existing);
		logger.info(std::format("Created copy at \"{}\"", copyPath.string()));
		
		filepath = g_config.inputFilepath;
	}
	else
		filepath = g_config.inputDir / (stem + ".map");

	logger.info("Converting func_map2prop entities");

	std::unordered_map<std::string, std::string> parentModels;
	for (M2PEntity::Entity& entity : entities)
	{
		if (entity.classname != "func_map2prop")
			continue;

		if (!entity.hasKey("targetname"))
			continue;

		if (parentModels.contains(entity.getKey("targetname")))
		{
			logger.info(std::format(
				"Naming conflict: Multiple func_map2prop entities with name \"{}\". "
				"Only the first one will be used as template parent",
				entity.getKey("targetname")
			));
			continue;
		}

		// Reset entity angles, as they're baked into the model now
		entity.setKey("angles", "0 0 0");

		parentModels[entity.getKey("targetname")] = entity.getKey("model");
	}

	logger.info("Writing modified MAP as " + filepath.string());

	std::ofstream file{ filepath };
	if (!file.good())
	{
		logger.error("Could not open \"" + filepath.string() + "\" for writing");
		return;
	}

	// Convert func_map2prop entites
	for (M2PEntity::Entity& entity : entities)
	{
		if (entity.classname != "func_map2prop")
		{
			file << entity.toString();
			continue;
		}

		// Entity is disabled, skip
		if (entity.getKeyInt("spawnflags") & 1)
			continue;

		if (entity.hasKey("parent_model"))
		{
			if (!parentModels.contains(entity.getKey("parent_model")))
			{
				logger.warning("");
				continue;
			}

			entity.setKey("model", parentModels.at(entity.getKey("parent_model")));
		}

		std::string newClass = entity.hasKey("convert_to") ? entity.getKey("convert_to") : "env_sprite";

		int spawnflags = 0;
		if (newClass.starts_with("monster_"))
			spawnflags |= 16; // Prisoner
		if (newClass == "monster_generic")
			spawnflags |= 4;  // Not solid

		file << "{\n\"classname\" \"" << newClass << "\"\n";
		file << "\"model\" \"" << entity.getKey("model") << "\"\n";
		file << "\"spawnflags\" \"" << spawnflags << "\"\n";

		if (entity.hasKey("targetname"))
			file << "\"targetname\" \"" << entity.getKey("targetname") << "\"\n";

		if (entity.hasKey("angles"))
		{
			std::vector<std::string> parts = M2PUtils::split(entity.getKey("angles"));
			file << "\"angles\" \"360 " << parts.at(1) << " 360\"\n";
		}

		if (entity.hasKey("origin"))
			file << "\"origin\" \"" << entity.getKey("origin") << "\"\n";

		file << "}\n";
	}

	logger.info("MAP successfully written. Ready for CSG");
}
