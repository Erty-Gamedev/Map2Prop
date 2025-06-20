#include <string>
#include <format>
#include <vector>
#include <array>
#include "rmf_format.h"
#include "logging.h"
#include "utils.h"
#include "binutils.h"

static inline Logging::Logger& logger = Logging::Logger::getLogger("rmfreader");

using namespace M2PRMF;
using namespace M2PFormat;
using namespace M2PGeo;
using namespace M2PEntity;
using namespace M2PBinUtils;


std::string RmfBrush::getRaw() const
{
	std::string raw = "{\n";
	raw.reserve(1024);

	const char* p = ".6g";
	size_t countFaces = faces.size();
	for (const auto& face : faces)
	{
		Vertex x = M2PUtils::getCircular(face.vertices, -1);
		Vertex y = M2PUtils::getCircular(face.vertices, -2);
		Vertex z = M2PUtils::getCircular(face.vertices, -3);
		FP ux = face.texture.rightaxis.x, uy = face.texture.rightaxis.y, uz = face.texture.rightaxis.z;
		FP vx = face.texture.downaxis.x, vy = face.texture.downaxis.y, vz = face.texture.downaxis.z;
		FP shiftx = face.texture.shiftx, shifty = face.texture.shifty;
		FP r = face.texture.angle, scalex = face.texture.scalex, scaley = face.texture.scaley;

		raw += std::format("( {:.6g} {:.6g} {:.6g} ) ",    x.x, x.y, x.z);
		raw += std::format("( {:.6g} {:.6g} {:.6g} ) ",    y.x, y.y, y.z);
		raw += std::format("( {:.6g} {:.6g} {:.6g} ) {} ", z.x, z.y, z.z, face.texture.name);

		raw += std::format("[ {:.6g} {:.6g} {:.6g} {:.6g} ] ", ux, uy, uz, shiftx);
		raw += std::format("[ {:.6g} {:.6g} {:.6g} {:.6g} ] ", vx, vy, vz, shifty);
		raw += std::format("{:.6g} {:.6g} {:.6g}\n",           r, scalex, scaley);
	}

	raw += "}\n";
	return raw;
}

std::string RmfEntity::toString() const
{
	std::string str;
	str.reserve(1024);
	str = "{\n";

	for (std::pair<std::string, std::string> const& pair : keyvalues)
	{
		str += std::format("\"{}\" \"{}\"\n", pair.first, pair.second);
	}

	if (!brushes.empty())
	{
		for (const std::unique_ptr<Brush>& brush : brushes)
		{
			str += brush->getRaw();
		}
	}

	str += "}\n";

	return str;
}

RmfReader::RmfReader(const std::filesystem::path& filepath, const std::filesystem::path& outputDir)
{
	m_filepath = filepath;
	m_outputDir = outputDir;
	m_file.open(filepath, std::ios::binary);
	if (!m_file.is_open() || !m_file.good())
	{
		m_file.close();
		logger.error("Could not open file " + filepath.string());
		exit(EXIT_FAILURE);
	}

	parse();
}
RmfReader::RmfReader(const std::filesystem::path& filepath, const std::filesystem::path& outputDir, int seekTo)
{
	m_filepath = filepath;
	m_outputDir = outputDir;
	m_file.open(filepath, std::ios::binary);
	if (!m_file.is_open() || !m_file.good())
	{
		m_file.close();
		logger.error("Could not open file " + filepath.string());
		exit(EXIT_FAILURE);
	}

	m_file.seekg(seekTo, std::ios::beg);
	parse();
}
RmfReader::~RmfReader()
{
	if (m_file.is_open())
		m_file.close();
}

void RmfReader::parse()
{
	RmfHeader header{};
	m_file.read(reinterpret_cast<char*>(&header), sizeof(RmfHeader));

	if (strncmp(header.magic, "RMF", 3))
	{
		logger.error(m_filepath.string() + " is not a valid RMF file");
		exit(EXIT_FAILURE);
	}

	m_version = static_cast<int>(round(header.version * 10));
	if (!M2PUtils::contains(c_SUPPORTED_VERSIONS, m_version))
	{
		logger.error(std::format("Unsupported RMF version: {}", m_version));
		exit(EXIT_FAILURE);
	}

	std::int32_t visgroupCount = readInt(m_file);
	for (int i = 0; i < visgroupCount; ++i)
		readVisgroup();

	entities.push_back(std::make_unique<RmfEntity>());
	auto& worldspawn = *entities[0];
	worldspawn.classname = "worldspawn";
	worldspawn.keyvalues.emplace_back("classname", worldspawn.classname);
	worldspawn.keyvalues.emplace_back("mapversion", "220");

	std::string objectType = readLPString(m_file); // CMapWorld
	MapObjectData cMapWorldData{};
	m_file.read(reinterpret_cast<char*>(&cMapWorldData), sizeof(MapObjectData));

	readChildren(cMapWorldData.childCount, worldspawn);

	std::string classname = readLPString(m_file); // "worldspawn"
	if (classname != "worldspawn")
		throw std::runtime_error("Expected worldspawn, but was \"" + classname + "\"");

	EntityData worldspawnData{};
	m_file.read(reinterpret_cast<char*>(&worldspawnData), sizeof(EntityData));

	std::string key, value;
	for (int i = 0; i < worldspawnData.kvCount; ++i)
	{
		key = readLPString(m_file);
		value = readLPString(m_file);
		worldspawn.keyvalues.emplace_back(key, value);
	}
	m_file.seekg(12, std::ios::cur); // Padding?

	if (worldspawnData.spawnflags)
		worldspawn.setKey("spawnflags", std::to_string(worldspawnData.spawnflags));

	std::int32_t pathCount = readInt(m_file);
	for (int i = 0; i < pathCount; ++i)
		readPath();

}

void RmfReader::readChildren(int count, Entity &parent)
{
	std::string objectType;

	for (int i = 0; i < count; ++i)
	{
		objectType = readLPString(m_file);

		if (objectType == "CMapSolid")
		{
			parent.brushes.push_back(std::make_unique<RmfBrush>());
			Brush& brush = *parent.brushes.back();
			readBrush(brush);
			continue;
		}
		if (objectType == "CMapEntity")
		{
			entities.push_back(std::make_unique<RmfEntity>());
			readEntity(*entities.back());
			continue;
		}
		if (objectType == std::string{ "CMapGroup" })
		{
			readGroup(parent);
			continue;
		}

		throw std::runtime_error("Invalid object type: \"" + objectType + "\"");
	}
}

void RmfReader::readVisgroup()
{
	Visgroup visgroup{};
	m_file.read(reinterpret_cast<char*>(&visgroup), sizeof(Visgroup));
}


void RmfReader::readEntity(Entity& entity)
{
	MapObjectData objectData{};
	m_file.read(reinterpret_cast<char*>(&objectData), sizeof(MapObjectData));

	readChildren(objectData.childCount, entity);

	entity.classname = readLPString(m_file);
	entity.setKey("classname", entity.classname);

	EntityData entData{};
	m_file.read(reinterpret_cast<char*>(&entData), sizeof(EntityData));

	std::string key, value;
	for (int i = 0; i < entData.kvCount; ++i)
	{
		key = readLPString(m_file);
		value = readLPString(m_file);
		entity.keyvalues.emplace_back(key, value);
	}

	m_file.seekg(14, std::ios::cur); // Padding?

	if (entData.spawnflags && !entity.hasKey("spawnflags"))
		entity.setKey("spawnflags", std::to_string(entData.spawnflags));

	float origin[3]{};
	m_file.read(reinterpret_cast<char*>(&origin), sizeof(origin));

	if (entity.brushes.empty())
		entity.setKey("origin", std::format("{:.6g} {:.6g} {:.6g}", origin[0], origin[1], origin[2]));

	m_file.seekg(4, std::ios::cur); // Padding?
}

void RmfReader::readBrush(Brush& brush)
{
	MapObjectData objectData{};
	m_file.read(reinterpret_cast<char*>(&objectData), sizeof(MapObjectData));
	//readChildren(objectData.childCount, parent);

	std::int32_t faceCount = readInt(m_file);
	for (int i = 0; i < faceCount; ++i)
	{
		Face face = readFace();
		brush.faces.push_back(face);
	}
}

M2PEntity::Face RmfReader::readFace()
{
	Face face;

	face.texture.name = (m_version < 18) ? readNTString(m_file, 40) : readNTString(m_file, 260);

	M2PWad3::ImageSize imageInfo = wadHandler.checkTexture(face.texture.name);

	face.texture.width = imageInfo.width;
	face.texture.height = imageInfo.height;

	if (m_version < 22)
	{
		face.texture.angle = readFloat(m_file);
		face.texture.shiftx = readFloat(m_file);
		face.texture.shifty = readFloat(m_file);
	}
	else
	{
		float rightaxis[3]{}, downaxis[3]{};
		m_file.read(reinterpret_cast<char*>(&rightaxis), sizeof(rightaxis));
		face.texture.shiftx = readFloat(m_file);
		m_file.read(reinterpret_cast<char*>(&downaxis), sizeof(downaxis));
		face.texture.shifty = readFloat(m_file);
		face.texture.angle = readFloat(m_file);

		face.texture.rightaxis = Vector3(rightaxis);
		face.texture.downaxis = Vector3(downaxis);
	}
	face.texture.scalex = readFloat(m_file);
	face.texture.scaley = readFloat(m_file);

	// Padding
	if (m_version < 18)
		m_file.seekg(4, std::ios::cur);
	else
		m_file.seekg(16, std::ios::cur);

	std::int32_t vertexCount = readInt(m_file);
	float coord[3]{};
	for (int i = 0; i < vertexCount; ++i)
	{
		m_file.read(reinterpret_cast<char*>(&coord), sizeof(coord));
		face.vertices.emplace_back(coord[0], coord[1], coord[2]);
	}
	std::reverse(face.vertices.begin(), face.vertices.end());

	float planepoints[3][3]{};
	m_file.read(reinterpret_cast<char*>(&planepoints), sizeof(planepoints));

	Vector3 normalPoints[3] = { Vector3(planepoints[2]), Vector3(planepoints[1]), Vector3(planepoints[0]) };
	face.normal = planeNormal(normalPoints);

	if (m_version < 22)
	{
		Vector3 normalPoints[3] = { Vector3(planepoints[2]), Vector3(planepoints[1]), Vector3(planepoints[0]) };
		Vector3 normal = planeNormal(normalPoints);
		FP vecs[2][3]{};
		int sv, tv;
		FP theta = deg2rad(face.texture.angle);
		FP sinv = sin(theta), cosv = cos(theta);
		textureAxisFromPlane(normal, vecs[0], vecs[1]);

		if (static_cast<int>(round(vecs[0][0]))) sv = 0;
		else if (static_cast<int>(round(vecs[0][1]))) sv = 1;
		else sv = 2;

		if (static_cast<int>(round(vecs[1][0]))) tv = 0;
		else if (static_cast<int>(round(vecs[1][1]))) tv = 1;
		else tv = 2;

		for (int i = 0; i < 2; ++i)
		{
			FP ns = cosv * vecs[i][sv] - sinv * vecs[i][tv];
			FP nt = sinv * vecs[i][sv] + cosv * vecs[i][tv];
			vecs[i][sv] = ns;
			vecs[i][tv] = nt;
		}

		face.texture.rightaxis = { vecs[0] };
		face.texture.downaxis = { vecs[1] };
	}

	for (M2PGeo::Vertex& vertex : face.vertices)
	{
		vertex.uv = face.texture.uvForPoint(vertex);
		vertex.normal = face.normal;
	}

	return face;
}

void RmfReader::readGroup(Entity &parent)
{
	MapObjectData objectData{};
	m_file.read(reinterpret_cast<char*>(&objectData), sizeof(MapObjectData));
	readChildren(objectData.childCount, parent);
}

void RmfReader::readPath()
{
	std::string name = readNTString(m_file, 128);
	std::string classname = readNTString(m_file, 128);
	std::int32_t pathType = readInt(m_file);
	std::int32_t nodeCount = readInt(m_file);
	for (int i = 0; i < nodeCount; ++i)
		readPathNode();
}

void RmfReader::readPathNode()
{
	float position[3]{};
	m_file.read(reinterpret_cast<char*>(&position), sizeof(position));
	std::int32_t index = readInt(m_file);
	std::string targetname = readNTString(m_file, 128);
	std::int32_t kvCount = readInt(m_file);
	std::string key, value;
	for (int i = 0; i < kvCount; ++i)
	{
		key = readLPString(m_file);
		value = readLPString(m_file);
	}
}


void M2PRMF::textureAxisFromPlane(const Vector3 normal, FP xvOut[3], FP yvOut[3])
{
	int bestaxis = 0;
	FP best = 0.;

	for (int i = 0; i < 6; ++i)
	{
		FP dot = normal.dot(Vector3{ c_BASE_AXIS[i * 3] });
		if (dot > best)
		{
			best = dot;
			bestaxis = i;
		}
	}

	for (int i = 0; i < 3; ++i)
	{
		xvOut[i] = c_BASE_AXIS[bestaxis * 3 + 1][i];
		yvOut[i] = c_BASE_AXIS[bestaxis * 3 + 2][i];
	}
}
