#include <cmath>
#include <fstream>
#include <filesystem>
#include <format>
#include <unordered_map>
#include "export.h"
#include "config.h"
#include "logging.h"
#include "utils.h"
#include "ear_clip.h"
#include "halfedge.h"


static inline Logging::Logger& logger = Logging::Logger::getLogger("export");
static inline const char* entProps[] = {
	"origin", "body", "rendermode", "renderamt", "rendercolor", "renderfx"
};


using M2PConfig::g_config;

using namespace M2PExport;
using namespace M2PGeo;
namespace fs = std::filesystem;


static inline void renameChrome(ModelData& model)
{
	if (!model.renameChrome)
		return;

	for (auto& pFace : model.mesh.faces)
	{
		if (!pFace)
			continue;

		std::string textureName = pFace->textureName;
		if (M2PUtils::toUpperCase(textureName).find("CHROME") == std::string::npos)
			continue;

		fs::path textureFilepath = g_config.extractDir() / (textureName + ".bmp");
		if (!fs::exists(textureFilepath)) // Check for lowercase version of the filename
			textureFilepath = g_config.extractDir() / (M2PUtils::toLowerCase(textureName) + ".bmp");
		if (!fs::exists(textureFilepath))
		{
			logger.warning("Could not rename file \"" + textureName + ".bmp\"" + ". File was not found");
			continue;
		}

		std::string newName = M2PUtils::toLowerCase(textureName);
		M2PUtils::replaceToken(newName, "chrome", "chrm");

		fs::copy_file(textureFilepath, g_config.extractDir() / (newName + ".bmp"), fs::copy_options::overwrite_existing);
		pFace->textureName = newName;
	}
}

static inline void applySmooth(ModelData& model)
{
	model.mesh.markSmoothEdges(model.smoothing, model.alwaysSmooth, model.neverSmooth);

	for (const auto& pVertex : model.mesh.coords)
		if (pVertex)
			model.mesh.getSmoothFansByVertex(*pVertex);
}


static inline int compileModel(const ModelData& model)
{
	fs::path currentDir = fs::absolute(fs::current_path());
	fs::current_path(fs::current_path() / g_config.extractDir());
#ifdef _WIN32
	std::string cmd = "\"\"" + g_config.studiomdl.string() + "\" \"" + model.outname + ".qc\"\"";
	logger.info("Running: " + cmd.substr(1, cmd.size() - 2));
#else
	std::string cmd = "\"" + g_config.studiomdl.string() + "\" \"" + model.outname + ".qc\"";
	logger.info("Running: " + cmd);
#endif
	int returnCode = std::system(cmd.c_str());

	std::cout.flush();
	fs::current_path(currentDir);
	return returnCode;
}


static inline void writeSmdFace(std::ofstream& file, std::array<M2PHalfEdge::Vertex, 3> vertices, bool flipped)
{
	if (flipped)
	{
		std::swap(vertices[0], vertices[1]);
		for (M2PHalfEdge::Vertex &vertex : vertices)
			vertex.normal = -vertex.normal;
	}

	for (const M2PHalfEdge::Vertex &vertex : vertices)
	{
		file << "0\t";
		const M2PGeo::Vector3& pos = vertex.position->coord();
		const M2PGeo::Vector3& normal = vertex.normal;

		if (g_config.isObj())
		{
			file << std::format("{:.6f} {:.6f} {:.6f}\t", pos.x, -pos.z, pos.y);
			file << std::format("{:.6f} {:.6f} {:.6f}\t", normal.x, -normal.z, normal.y);
			file << std::format("{:.6f} {:.6f}", vertex.uv.x, vertex.uv.y + 1);
		}
		else
		{
			file << std::format("{:.6f} {:.6f} {:.6f}\t", pos.x, pos.y, pos.z);
			file << std::format("{:.6f} {:.6f} {:.6f}\t", normal.x, normal.y, normal.z);
			file << std::format("{:.6f} {:.6f}", vertex.uv.x, vertex.uv.y + 1);
		}
		file << "\n";
	}
}

static inline bool writeSmd(const ModelData& model)
{
	fs::path filepath = g_config.extractDir() / (model.outname + ".smd");
	std::ofstream file{ filepath };
	if (!file.is_open() || !file.good())
	{
		file.close();
		logger.warning("Could not open file for writing: " + filepath.string());
		return false;
	}

	logger.debug("Writing " + filepath.string());

	file << "version 1\nnodes\n0 \"root\" -1\nend\nskeleton\ntime 0\n0 0 0 0 0 0 0\nend\ntriangles\n";

	for (const auto& pFace : model.mesh.faces)
	{
		if (!pFace)
			continue;

		file << M2PUtils::toLowerCase(pFace->textureName) << ".bmp\n";
		writeSmdFace(file, pFace->vertices, false);
		if (pFace->flipped)
		{
			file << M2PUtils::toLowerCase(pFace->textureName) << ".bmp\n";
			writeSmdFace(file, pFace->vertices, true);
		}
	}
	file << "end\n";

	bool res = file.good();
	file.close();

	if (!res)
	{
		logger.error("Something went wrong when writing to " + filepath.string());
		return false;
	}

	logger.debug("Successfully written " + filepath.string());
	return true;
}

static inline bool writeQc(const ModelData& model)
{
	if (!model.parent.empty())
		return true;

	fs::path filepath = g_config.extractDir() / (model.outname + ".qc");
	std::ofstream file{ filepath };
	if (!file.is_open() || !file.good())
	{
		file.close();
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
			rendermodes += "$texrendermode " + M2PUtils::toLowerCase(masked) + ".bmp masked\n";
	}

	Vector3 qcOffset{ g_config.qcOffset[0], g_config.qcOffset[1], g_config.qcOffset[2] };
	if (qcOffset != Vector3::zero())
		offset = std::format("{:.6g} {:.6g} {:.6g}", qcOffset.x, qcOffset.y, qcOffset.z);

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

	std::string subdir = model.subdir.empty() ? "" : model.subdir + "/";

	file << "/*\n Automatically generated by Erty's GoldSrc Map2Prop.\n*/\n\n";
	file << "$modelname " << subdir << model.outname << ".mdl\n";
	file << "$cd \".\"\n$cdtexture \".\"\n";
	file << "$scale " << model.scale << "\n";
	file << "$origin " << offset << " " << model.rotation << "\n";
	file << qcFlags << rendermodes << bbox << cbox << "$gamma " << g_config.qcGamma << "\n";

	if (model.submodels.empty())
		file << "$body studio \"" << model.outname << "\"\n";
	else
	{
		file << "$bodygroup \"body\"\n{\n";
		file << "\tstudio \"" + model.outname + "\"\n";
		for (const auto& submodel : model.submodels)
		{
			file << "\tstudio \"" + submodel + "\"\n";
		}
		file << "}\n";
	}

	file << "$sequence \"Generated_with_Erty's_Map2Prop\" \"" << model.outname << "\"\n";

	bool res = file.good();
	file.close();

	if (!res)
	{
		logger.error("Something went wrong when writing to " + filepath.string());
		return false;
	}

	logger.debug("Successfully written " + filepath.string());
	return true;
}

static inline void generateClip(std::ofstream& file, M2PEntity::Entity& entity, const std::unordered_map<std::string, M2PEntity::Entity*>& parentEntities)
{
	int clipGenType = entity.getKeyInt("clip_type");
	if (clipGenType == 0) return;

	Bounds bb;
	FP scale;
	if (entity.hasKey("parent_model") && !(entity.getKeyInt("spawnflags") & Spawnflags::IS_SUBMODEL))
	{
		M2PEntity::Entity& parent = *parentEntities.at(entity.getKey("parent_model"));
		bb = parent.getBounds();

		M2PGeo::Vector3 eOrigin, pOrigin, offset;
		std::vector<std::string> eParts = M2PUtils::split(entity.getKey("origin"));
		if (eParts.size() == 3)
			eOrigin = { std::stof(eParts[0]), std::stof(eParts[1]), std::stof(eParts[2]) };

		std::vector<std::string> pParts = M2PUtils::split(parent.getKey("origin"));
		if (pParts.size() == 3)
			pOrigin = { std::stof(pParts[0]), std::stof(pParts[1]), std::stof(pParts[2]) };

		offset = eOrigin - pOrigin;
		bb.min += offset;
		bb.max += offset;

		scale = parent.getKeyFloat("scale");
	}
	else
	{
		bb = entity.getBounds();
		scale = entity.getKeyFloat("scale");
	}

	if (scale != .0 && scale != 1.)
	{
		bb.min *= scale;
		bb.max *= scale;
	}

	Vector3 boundsSize = bb.getSize();
	if (boundsSize.x < g_config.clipThreshold && boundsSize.y < g_config.clipThreshold && boundsSize.z < g_config.clipThreshold)
		return;

	file << "{\n\"classname\" \"func_detail\"\n"
		<< "\"zhlt_detaillevel\" \"0\"\n\"zhlt_chopdown\" \"0\"\n"
		<< "\"zhlt_chopup\" \"0\"\n\"zhlt_coplanarpriority\" \"1\"\n"
		<< "\"zhlt_clipnodedetaillevel\" \"1\"\n{\n";

	switch (clipGenType)
	{
	case ClipGenType::BOX:
		file << std::format(
			"( {3:.6g} {4:.6g} {5:.6g} ) ( {3:.6g} {4:.6g} {2:.6g} ) ( {3:.6g} {1:.6g} {5:.6g} ) CLIP [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"\
			"( {0:.6g} {1:.6g} {5:.6g} ) ( {0:.6g} {1:.6g} {2:.6g} ) ( {0:.6g} {4:.6g} {5:.6g} ) CLIP [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"\
			"( {3:.6g} {1:.6g} {5:.6g} ) ( {3:.6g} {1:.6g} {2:.6g} ) ( {0:.6g} {1:.6g} {5:.6g} ) CLIP [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"\
			"( {0:.6g} {4:.6g} {5:.6g} ) ( {0:.6g} {4:.6g} {2:.6g} ) ( {3:.6g} {4:.6g} {5:.6g} ) CLIP [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"\
			"( {0:.6g} {4:.6g} {2:.6g} ) ( {0:.6g} {1:.6g} {2:.6g} ) ( {3:.6g} {4:.6g} {2:.6g} ) CLIP [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1\n"\
			"( {3:.6g} {1:.6g} {5:.6g} ) ( {0:.6g} {1:.6g} {5:.6g} ) ( {3:.6g} {4:.6g} {5:.6g} ) CLIP [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1\n",
			bb.min.x, bb.min.y, bb.min.z, bb.max.x, bb.max.y, bb.max.z);
		break;
	case ClipGenType::CYLINDER:
		Bounds bc{ bb.min * c_SIN45, bb.max * c_SIN45 };
		Vector3 center = (bb.min + bb.max) / 2;

		file << std::format(
			"( {12:.6g} {1:.6g} {2:.6g} ) ( {9:.6g} {7:.6g} {2:.6g} ) ( {6:.6g} {7:.6g} {2:.6g} ) CLIP [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1\n"\
			"( {9:.6g} {10:.6g} {5:.6g} ) ( {3:.6g} {13:.6g} {5:.6g} ) ( {12:.6g} {4:.6g} {5:.6g} ) CLIP [ 1 0 0 0 ] [ 0 -1 0 0 ] 0 1 1\n"\
			"( {9:.6g} {7:.6g} {2:.6g} ) ( {9:.6g} {7:.6g} {5:.6g} ) ( {3:.6g} {13:.6g} {2:.6g} ) CLIP [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"\
			"( {12:.6g} {1:.6g} {2:.6g} ) ( {12:.6g} {1:.6g} {5:.6g} ) ( {9:.6g} {7:.6g} {2:.6g} ) CLIP [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"\
			"( {6:.6g} {7:.6g} {2:.6g} ) ( {6:.6g} {7:.6g} {5:.6g} ) ( {12:.6g} {1:.6g} {2:.6g} ) CLIP [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"\
			"( {0:.6g} {13:.6g} {2:.6g} ) ( {0:.6g} {13:.6g} {5:.6g} ) ( {6:.6g} {7:.6g} {2:.6g} ) CLIP [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"\
			"( {6:.6g} {10:.6g} {2:.6g} ) ( {6:.6g} {10:.6g} {5:.6g} ) ( {0:.6g} {13:.6g} {2:.6g} ) CLIP [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"\
			"( {12:.6g} {4:.6g} {2:.6g} ) ( {12:.6g} {4:.6g} {5:.6g} ) ( {6:.6g} {10:.6g} {2:.6g} ) CLIP [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"\
			"( {9:.6g} {10:.6g} {2:.6g} ) ( {9:.6g} {10:.6g} {5:.6g} ) ( {12:.6g} {4:.6g} {2:.6g} ) CLIP [ 1 0 0 0 ] [ 0 0 -1 0 ] 0 1 1\n"\
			"( {3:.6g} {13:.6g} {2:.6g} ) ( {3:.6g} {13:.6g} {5:.6g} ) ( {9:.6g} {10:.6g} {2:.6g} ) CLIP [ 0 1 0 0 ] [ 0 0 -1 0 ] 0 1 1\n",
			bb.min.x, bb.min.y, bb.min.z, bb.max.x, bb.max.y, bb.max.z,
			bc.min.x, bc.min.y, bc.min.z, bc.max.x, bc.max.y, bc.max.z,
			center.x, center.y, center.z);
		break;
	}

	file << "}\n}\n";
}


std::unordered_map<std::string, ModelData> M2PExport::prepareModels(M2PEntity::BaseReader& reader, const std::string& _filename)
{
	int n = 0;
	std::unordered_map<std::string, ModelData> modelsMap;
	std::unordered_map<std::string, unsigned int> submodelIndices;
	std::string keyvalue;
	keyvalue.reserve(256);

	for (std::unique_ptr<M2PEntity::Entity>& entity : reader.entities)
	{
		bool isWorldspawn = entity->classname == "worldspawn";
		bool isFuncM2P = entity->classname == "func_map2prop";

		if (isWorldspawn)
		{
			if (!g_config.isMap())
			{
				std::string wads;
				wads.reserve(1024);
				for (int i = 0; i < reader.wadHandler.usedWads.size(); ++i) {
					fs::path wadPath = reader.wadHandler.usedWads[i];
					wadPath = fs::absolute(wadPath);
					std::string wadStr = "/" + fs::relative(wadPath, wadPath.root_path()).string();
					std::replace(wadStr.begin(), wadStr.end(), '\\', '/');
					wads.append(wadStr);
					if (i < reader.wadHandler.usedWads.size() - 1)
						wads.append(";");
				}
				entity->keyvalues.emplace_back("wad", wads);
			}
			entity->setKey(c_NOTE_KEY, c_NOTE_VALUE);
		}

		if (entity->brushes.empty())
			continue;

		if (g_config.mapcompile && !isFuncM2P)
			continue;

		std::string filename = _filename.empty() ? g_config.inputFilepath.stem().string() : _filename;
		std::string outname = !g_config.outputName.empty() ? g_config.outputName : filename;
		bool ownModel = false;
		std::string parent = "";
		std::string subdir = "";

		if (isFuncM2P)
		{
			if (entity->getKeyInt("spawnflags") & Spawnflags::DISABLE)
				continue;

			keyvalue = entity->getKey("parent_model");
			if (!keyvalue.empty())
			{
				if (entity->getKeyInt("spawnflags") & Spawnflags::IS_SUBMODEL)
				{
					parent = keyvalue;
					if (!submodelIndices.contains(keyvalue))
						submodelIndices[keyvalue] = 1;
					else
						++submodelIndices[keyvalue];
					entity->setKey("body", std::to_string(submodelIndices[keyvalue]));
				}
				else
				{
					for (const auto& brush : entity->brushes)
					{
						if (brush->faces.empty())
							continue;

						if (brush->isToolBrush(M2PEntity::ToolTexture::ORIGIN))
						{
							Vector3 ori = brush->getCenter();
							entity->setKey("origin", std::format("{:.6g} {:.6g} {:.6g}", ori.x, ori.y, ori.z));
							break;
						}
					}
					continue;
				}
			}

			if (g_config.mapcompile || entity->getKeyInt("own_model") > 0)
			{
				ownModel = true;
				outname = std::format("{}_{}", filename, n);
				keyvalue = entity->getKey("outname");
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

			keyvalue = entity->getKey("subdir");
			if (!keyvalue.empty())
				subdir = keyvalue;

			fs::path parentFolder = g_config.extractDir() / subdir;

			if (!fs::is_directory(parentFolder))
				fs::create_directories(parentFolder);

			std::string modelPath = ("models" / g_config.outputDir / subdir / (outname + ".mdl")).string();
			std::replace(modelPath.begin(), modelPath.end(), '\\', '/');
			entity->setKey("model", modelPath);
		}


		float scale = g_config.qcScale;
		float rotation = g_config.qcRotate;
		float smoothing = g_config.smoothing;
		bool chrome = g_config.renameChrome;
		std::string qcFlags = "";

		if (isWorldspawn || ownModel)
		{
			if (!(keyvalue = entity->getKey("scale")).empty())
			{
				scale = std::stof(keyvalue);
				if (scale == 0)
					scale = 1.0;
			}

			if (!(keyvalue = entity->getKey("angles")).empty())
			{
				std::vector<std::string> angles = M2PUtils::split(keyvalue, ' ');
				if (angles.size() == 1)
					rotation = fmod(rotation + std::stof(angles[0]), 360.0f);
				else if (angles.size() > 2)
					rotation = fmod(rotation + std::stof(angles[1]), 360.0f);
			}

			if (!(keyvalue = entity->getKey("smoothing")).empty())
				smoothing = std::stof(keyvalue);

			if (!(keyvalue = entity->getKey("qc_flags")).empty())
				qcFlags = keyvalue;

			chrome = entity->getKeyInt("chrome") == 1 || entity->getKeyInt("spawnflags") & Spawnflags::RENAME_CHROME;
		}

		
		if (!modelsMap.contains(outname))
		{
			modelsMap[outname].targetname = entity->getKey("targetname");
			modelsMap[outname].outname = outname;
			modelsMap[outname].subdir = subdir;
			modelsMap[outname].scale = scale;
			modelsMap[outname].rotation = rotation;
			modelsMap[outname].smoothing = smoothing;
			modelsMap[outname].renameChrome = chrome;
			modelsMap[outname].qcFlags = qcFlags;
			modelsMap[outname].parent = parent;
		}

		bool originFound = false, boundsFound = false, clipFound = false;
		for (const auto& brush : entity->brushes)
		{

			// Look for ORIGIN brushes, use first found
			if (modelsMap[outname].offset == Vector3::zero() && brush->isToolBrush(M2PEntity::ToolTexture::ORIGIN))
			{
				if (originFound)
				{
					logger.info(std::format("Multiple ORIGIN brushes found in {} near ({})", entity->classname, brush->getCenter()));
					continue;
				}
				if (isWorldspawn || ownModel)
				{
					Vector3 origin = geometricCenter(brush->getBounds());
					modelsMap[outname].offset = origin;
					entity->setKey("origin", std::format("{}", origin));
				}
				originFound = true;
				continue;
			}

			// Look for BOUNDINGBOX brushes, use first found
			if (modelsMap[outname].bounds == Bounds::zero()
				&& brush->isToolBrush(M2PEntity::ToolTexture::BOUNDINGBOX))
			{
				if (boundsFound)
				{
					logger.info(std::format("Multiple BOUNDINGBOX brushes found in {} near ({})", entity->classname, brush->getCenter()));
					continue;
				}
				if (isWorldspawn || ownModel)
				{
					modelsMap[outname].bounds = brush->getBounds();
				}
				boundsFound = true;
				continue;
			}
			
			// Look for CLIP brushes, use first found
			if (modelsMap[outname].clip == Bounds::zero()
				&& brush->isToolBrush(M2PEntity::ToolTexture::CLIP))
			{
				if (clipFound)
				{
					logger.info(std::format("Multiple CLIP brushes found in {} near ({})", entity->classname, brush->getCenter()));
					continue;
				}
				if (isWorldspawn || ownModel)
				{
					modelsMap[outname].clip = brush->getBounds();
				}
				clipFound = true;
				continue;
			}
			
			// Look for CLIPBEVEL brushes
			if (brush->isToolBrush(M2PEntity::ToolTexture::CLIPBEVEL))
			{
				if (isWorldspawn || ownModel)
				{
					modelsMap[outname].neverSmooth.push_back(brush->getBounds());
				}
				continue;
			}
			
			// Look for BEVEL brushes
			if (brush->isToolBrush(M2PEntity::ToolTexture::BEVEL))
			{
				if (isWorldspawn || ownModel)
				{
					modelsMap[outname].alwaysSmooth.push_back(brush->getBounds());
				}
				continue;
			}

			bool hasContentWater = brush->hasContentWater();

			for (const M2PEntity::Face& face : brush->faces)
			{
				if (M2PWad3::Wad3Handler::isSkipTexture(face.texture.name) || M2PWad3::Wad3Handler::isToolTexture(face.texture.name))
					continue;

				if (face.texture.name.starts_with('{'))
					modelsMap[outname].maskedTextures.insert(face.texture.name);

				ModelData& currentModel = modelsMap[outname];

				const std::vector<Triangle> triangles = earClip(face.vertices, face.normal);

				for (const Triangle& triangle : triangles)
					currentModel.mesh.addTriangle(triangle, face.texture, hasContentWater);
			}
		}

		if (modelsMap[outname].offset == Vector3::zero() && !entity->getKeyInt("use_world_origin"))
		{
			Vector3 aabbMin = modelsMap[outname].mesh.coords[0]->coord();
			Vector3 aabbMax = modelsMap[outname].mesh.coords[0]->coord();

			for (const auto& vertex : modelsMap[outname].mesh.coords)
			{
				if (vertex->x < aabbMin.x) aabbMin.x = vertex->x;
				if (vertex->y < aabbMin.y) aabbMin.y = vertex->y;
				if (vertex->z < aabbMin.z) aabbMin.z = vertex->z;

				if (vertex->x > aabbMax.x) aabbMax.x = vertex->x;
				if (vertex->y > aabbMax.y) aabbMax.y = vertex->y;
				if (vertex->z > aabbMax.z) aabbMax.z = vertex->z;
			}

			modelsMap[outname].offset = geometricCenter(std::vector{ aabbMin, aabbMax });
			modelsMap[outname].offset.z -= (aabbMax.z - aabbMin.z) / 2;

			Vector3& ori = modelsMap[outname].offset;
			entity->setKey("origin", std::format("{:.6g} {:.6g} {:.6g}", ori.x, ori.y, ori.z));
		}
	}

	for (const auto& kv : modelsMap)
	{
		const auto& model = kv.second;
		if (!model.parent.empty())
		{
			for (auto& kv2 : modelsMap)
			{
				auto& other = kv2.second;
				if (model.parent == other.targetname)
					other.submodels.push_back(model.outname);
			}
		}
	}

	return modelsMap;
}

int M2PExport::processModels(std::unordered_map<std::string, ModelData>& models, bool missingTextures, std::vector<fs::path>& successes)
{
	int returnCodes = 0;

	logger.debug("Processing %u model%c", models.size(), models.size() == 1 ? '\0' : 's');

	for (auto& kv : models)
	{
		ModelData& model = kv.second;

		renameChrome(model);
		applySmooth(model);

		model.applyOffset();

		if (!writeSmd(model))
			return 1;

		if (!writeQc(model))
			return 1;
	}

	logger.info("Finished processing %u model%c", models.size(), models.size() == 1 ? '\0' : 's');

	if (!g_config.autocompile)
		return 0;

	logger.info("Autocompile enabled, compiling model%c...", models.size() == 1 ? '\0' : 's');

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

	for (auto& kv : models)
	{
		ModelData& model = kv.second;

		if (!model.parent.empty())
			continue;

		int returnCode = compileModel(model);

		if (!returnCode)
			successes.emplace_back(fs::path{ model.subdir } / (model.outname + ".mdl"));
		returnCodes += returnCode;
	}

	return returnCodes;
}

void M2PExport::rewriteMap(std::vector<std::unique_ptr<M2PEntity::Entity>> &entities)
{
	std::string stem = g_config.inputFilepath.stem().string();

	fs::path filepath;
	if (g_config.isMap())
	{
		// TODO: Only make a M2P copy if we're working on an unmodified MAP
		fs::path copyPath = g_config.inputDir / (stem + ".m2p");
		fs::copy_file(g_config.inputFilepath, copyPath, fs::copy_options::overwrite_existing);
		logger.info(std::format("Created copy at \"{}\"", copyPath.string()));
		
		filepath = g_config.inputFilepath;
	}
	else
		filepath = g_config.inputDir / (stem + ".map");

	logger.info("Converting func_map2prop entities");

	std::unordered_map<std::string, M2PEntity::Entity*> parentEntities;
	for (std::unique_ptr<M2PEntity::Entity> &entity : entities)
	{
		if (entity->classname != "func_map2prop")
			continue;

		if (!entity->hasKey("targetname"))
			continue;

		if (parentEntities.contains(entity->getKey("targetname")))
		{
			logger.info(std::format(
				"Naming conflict: Multiple func_map2prop entities with name \"{}\". "
				"Only the first one will be used as template/submodel parent",
				entity->getKey("targetname")
			));
			continue;
		}

		// Reset entity angles, as they're baked into the model now
		entity->setKey("angles", "0 0 0");

		parentEntities[entity->getKey("targetname")] = entity.get();
	}

	logger.info("Writing modified MAP as " + filepath.string());

	std::ofstream file{ filepath };
	if (!file.good())
	{
		logger.error("Could not open \"" + filepath.string() + "\" for writing");
		return;
	}

	// Convert func_map2prop entites
	for (std::unique_ptr<M2PEntity::Entity> &entity : entities)
	{
		if (entity->classname != "func_map2prop")
		{
			file << entity->toString();
			continue;
		}

		// Entity is disabled, skip
		if (entity->getKeyInt("spawnflags") & Spawnflags::DISABLE)
			continue;

		if (entity->hasKey("parent_model"))
		{
			std::string parent = entity->getKey("parent_model");

			if (!parentEntities.contains(parent))
			{
				logger.warning("Parent model with targetname \"" + parent + "\" not found");
				continue;
			}

			entity->setKey("model", parentEntities.at(entity->getKey("parent_model"))->getKey("model"));
		}

		generateClip(file, *entity, parentEntities);


		std::string newClass = entity->hasKey("convert_to") ? entity->getKey("convert_to") : "env_sprite";

		int spawnflags = 0;
		if (newClass.starts_with("monster_"))
			spawnflags |= 16; // Prisoner
		if (newClass == "monster_generic")
			spawnflags |= 4;  // Not solid

		file << "{\n\"classname\" \"" << newClass << "\"\n";
		file << "\"model\" \"" << entity->getKey("model") << "\"\n";

		if (spawnflags)
			file << "\"spawnflags\" \"" << spawnflags << "\"\n";

		if (entity->hasKey("targetname"))
			file << "\"targetname\" \"" << entity->getKey("targetname") << "\"\n";

		if (entity->hasKey("angles"))
		{
			std::vector<std::string> parts = M2PUtils::split(entity->getKey("angles"));
			file << "\"angles\" \"360 " << parts.at(1) << " 360\"\n";
		}

		for (const auto& renderProp : entProps)
			if (entity->hasKey(renderProp))
				file << "\"" << renderProp << "\" \"" << entity->getKey(renderProp) << "\"\n";

		file << "}\n";
	}

	logger.info("MAP successfully written. Ready for CSG");
}
