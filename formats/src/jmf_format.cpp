#include <string>
#include <cstring>
#include <vector>
#include "jmf_format.h"
#include "logging.h"
#include "utils.h"
#include "binutils.h"

static inline Logging::Logger& logger = Logging::Logger::getLogger("jmfreader");

using namespace M2PJMF;
using namespace M2PFormat;
using namespace M2PGeo;
using namespace M2PEntity;
using namespace M2PBinUtils;


std::string JmfBrush::getRaw() const
{
	std::string raw = "{\n";
	raw.reserve(1024);

	for (const auto& face : faces)
	{
		Vertex x = M2PUtils::getCircular(face.vertices, -1);
		Vertex y = M2PUtils::getCircular(face.vertices, -2);
		Vertex z = M2PUtils::getCircular(face.vertices, -3);
		FP ux = face.texture.rightaxis.x, uy = face.texture.rightaxis.y, uz = face.texture.rightaxis.z;
		FP vx = face.texture.downaxis.x, vy = face.texture.downaxis.y, vz = face.texture.downaxis.z;
		FP shiftx = face.texture.shiftx, shifty = face.texture.shifty;
		FP r = face.texture.angle, scalex = face.texture.scalex, scaley = face.texture.scaley;

		raw += std::format("( {:.6g} {:.6g} {:.6g} ) ", x.x, x.y, x.z);
		raw += std::format("( {:.6g} {:.6g} {:.6g} ) ", y.x, y.y, y.z);
		raw += std::format("( {:.6g} {:.6g} {:.6g} ) {} ", z.x, z.y, z.z, face.texture.name);

		raw += std::format("[ {:.6g} {:.6g} {:.6g} {:.6g} ] ", ux, uy, uz, shiftx);
		raw += std::format("[ {:.6g} {:.6g} {:.6g} {:.6g} ] ", vx, vy, vz, shifty);
		raw += std::format("{:.6g} {:.6g} {:.6g}\n", r, scalex, scaley);
	}

	raw += "}\n";
	return raw;
}

std::string JmfEntity::toString() const
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

JmfReader::JmfReader(const std::filesystem::path& filepath, const std::filesystem::path& outputDir)
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
	m_file.close();
}
JmfReader::~JmfReader()
{
	if (m_file.is_open())
		m_file.close();
}

void JmfReader::parse()
{
	char magic[4]{};
	m_file.read(reinterpret_cast<char*>(&magic), sizeof(magic));

	if (strncmp(magic, "JHMF", 4))
	{
		logger.error(m_filepath.string() + " is not a valid JMF file");
		exit(EXIT_FAILURE);
	}

	m_version = readInt(m_file);
	if (!M2PUtils::contains(c_SUPPORTED_VERSIONS, m_version))
	{
		logger.error(std::format("Unsupported JMF version: {}", m_version));
		exit(EXIT_FAILURE);
	}

	std::int32_t exportPathCount = readInt(m_file);
	for (int i = 0; i < exportPathCount; ++i)
		readIntLPString(m_file);

	if (m_version >= 122)
	{
		for (int i = 0; i < 3; ++i)
			readBgImage();
	}

	std::int32_t groupCount = readInt(m_file);
	for (int i = 0; i < groupCount; ++i)
		readGroup();
	
	std::int32_t visgroupCount = readInt(m_file);
	for (int i = 0; i < visgroupCount; ++i)
		readVisgroup();

	float cordon[2][3]{};
	m_file.read(reinterpret_cast<char*>(&cordon), sizeof(cordon));

	std::int32_t cameraCount = readInt(m_file);
	for (int i = 0; i < cameraCount; ++i)
		readCamera();
	
	std::int32_t pathCount = readInt(m_file);
	for (int i = 0; i < pathCount; ++i)
		readPath();

	while (m_file.peek(), !m_file.eof())
		readEntity();
}

void JmfReader::readBgImage()
{
	std::string path = readIntLPString(m_file);
	JmfBgImage bgImage{};
	m_file.read(reinterpret_cast<char*>(&bgImage), sizeof(JmfBgImage));
}

void JmfReader::readGroup()
{
	JmfGroup group{};
	m_file.read(reinterpret_cast<char*>(&group), sizeof(JmfGroup));
}

void JmfReader::readVisgroup()
{
	std::string name = readIntLPString(m_file);
	JmfVisgroup visgroup{};
	m_file.read(reinterpret_cast<char*>(&visgroup), sizeof(JmfVisgroup));
}

void JmfReader::readCamera()
{
	JmfCamera camera{};
	m_file.read(reinterpret_cast<char*>(&camera), sizeof(JmfCamera));
}

void JmfReader::readPath()
{
	readIntLPString(m_file);		// classname
	readIntLPString(m_file);		// path name
	readInt(m_file);				// path type
	readInt(m_file);		    	// flags
	m_file.seekg(4, std::ios::cur); // color
	std::int32_t nodeCount = readInt(m_file);
	for (int i = 0; i < nodeCount; ++i)
		readPathNode();
}

void JmfReader::readPathNode()
{
	readIntLPString(m_file);		// name override
	readIntLPString(m_file);		// fire on pass
	m_file.seekg(sizeof(float[3]), std::ios::cur); // position
	m_file.seekg(sizeof(float[3]), std::ios::cur); // angles
	readInt(m_file);				// flags
	std::int32_t kvCount = readInt(m_file);
	for (int i = 0; i < kvCount; ++i)
	{
		readIntLPString(m_file); // key
		readIntLPString(m_file); // value
	}
}

void JmfReader::readEntity()
{
	entities.emplace_back(std::make_unique<JmfEntity>());
	Entity& entity = *entities.back();

	entity.classname = readIntLPString(m_file);
	entity.keyvalues.emplace_back("classname", entity.classname);

	if (entity.classname == "worldspawn")
		entity.keyvalues.emplace_back("mapversion", "220");

	JmfEntityHeader header{};
	m_file.read(reinterpret_cast<char*>(&header), sizeof(JmfEntityHeader));
	
	// Special JACK attributes, ignore
	for (int i = 0; i < 13; ++i)
		readIntLPString(m_file);

	JmfEntityBody body{};
	m_file.read(reinterpret_cast<char*>(&body), sizeof(JmfEntityBody));

	std::string key, value;
	for (int i = 0; i < body.kvCount; ++i)
	{
		key = readIntLPString(m_file);
		value = readIntLPString(m_file);
		entity.keyvalues.emplace_back(key, value);
	}

	std::int32_t visgroupCount = readInt(m_file);
	for (int i = 0; i < visgroupCount; ++i)
		readInt(m_file); // visgroup id

	std::int32_t brushCount = readInt(m_file);
	for (int i = 0; i < brushCount; ++i)
		readBrush(entity);

	if (!brushCount && !entity.hasKey("origin"))
		entity.keyvalues.emplace_back("origin",
			std::format("{:.6g} {:.6g} {:.6g}",
			header.origin[0], header.origin[1], header.origin[2])
		);
}

void JmfReader::readBrush(Entity& parent)
{
	parent.brushes.emplace_back(std::make_unique<JmfBrush>());
	Brush& brush = *parent.brushes.back();

	JmfBrushHeader header{};
	m_file.read(reinterpret_cast<char*>(&header), sizeof(JmfBrushHeader));

	for (int i = 0; i < header.visgroupCount; ++i)
		readInt(m_file); // visgroup id

	std::int32_t faceCount = readInt(m_file);
	for (int i = 0; i < faceCount; ++i)
		brush.faces.push_back(readFace());

	for (int i = 0; i < header.curveCount; ++i)
		readCurve();
}

void JmfReader::readCurve()
{
	//JmfCurve curve{};
	//m_file.read(reinterpret_cast<char*>(&curve), sizeof(JmfCurve));
	m_file.seekg(sizeof(JmfCurve), std::ios::cur); // Don't read, just skip
}

Face JmfReader::readFace()
{
	Face face;

	readInt(m_file); // Editor flags
	std::int32_t vertexCount = readInt(m_file);

	JmfFace faceProperties{};
	m_file.read(reinterpret_cast<char*>(&faceProperties), sizeof(JmfFace));
	faceProperties.toTexture(face.texture);

	M2PWad3::ImageSize imageInfo = wadHandler.checkTexture(face.texture.name);

	face.texture.width = imageInfo.width;
	face.texture.height = imageInfo.height;

	float normal[3]{};
	m_file.read(reinterpret_cast<char*>(&normal), sizeof(normal));
	face.normal = Vector3{ normal };

	readFloat(m_file); // Distance
	readInt(m_file); // Aligned axis (0=X, 1=Y, 2=Z, 3=Unaligned)

	for (int i = 0; i < vertexCount; ++i)
	{
		JmfVertex vertex{};
		m_file.read(reinterpret_cast<char*>(&vertex), sizeof(JmfVertex));
		face.vertices.emplace_back(vertex.coords, vertex.uv, face.normal);
	}

	return face;
}
