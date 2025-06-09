#include <string>
#include "obj_format.h"
#include "logging.h"
#include "utils.h"


static inline Logging::Logger& logger = Logging::Logger::getLogger("objreader");

using namespace M2PFormat;
using namespace M2POBJ;
using namespace M2PGeo;
using namespace M2PEntity;


ObjReader::ObjReader(const std::filesystem::path& filepath, const std::filesystem::path& outputDir)
{
	m_filepath = filepath;
	m_outputDir = outputDir;
	m_file.open(filepath);
	if (!m_file.is_open() || !m_file.good())
	{
		m_file.close();
		logger.error("Could not open file " + filepath.string());
		exit(EXIT_FAILURE);
	}

	parse();
}
ObjReader::~ObjReader()
{
	if (m_file.is_open())
		m_file.close();
}

void ObjReader::parse()
{
	std::string line;
	line.reserve(1024);
	std::getline(m_file, line);

	while (!line.empty())
	{
		if (line.starts_with('#'))
		{
			std::getline(m_file, line);
			continue;
		}

		// Parse coordinates
		if (line.starts_with(c_VERTEX_PREFIX))
		{
			line.replace(0, c_VERTEX_PREFIX.length(), "");
			m_vertexCoords.push_back(readCoordinate(line));
			std::getline(m_file, line);
			continue;
		}
		if (line.starts_with(c_UV_PREFIX))
		{
			line.replace(0, c_UV_PREFIX.length(), "");
			Vector3 coord = readCoordinate(line);
			m_uvCoords.emplace_back(coord.x, coord.y);
			std::getline(m_file, line);
			continue;
		}
		if (line.starts_with(c_NORMAL_PREFIX))
		{
			line.replace(0, c_NORMAL_PREFIX.length(), "");
			m_normalCoords.push_back(readCoordinate(line));
			std::getline(m_file, line);
			continue;
		}

		// Parse entities
		if (line.starts_with(c_OBJECT_PREFIX))
		{
			line.replace(0, c_OBJECT_PREFIX.length(), "");
			readEntity(line);
			continue;
		}

		std::getline(m_file, line);
	}
}

Vector3 ObjReader::readCoordinate(const std::string& line)
{
	const std::vector<std::string>& parts = M2PUtils::split(line, ' ');
	if (parts.size() != 3)
	{
		logger.error("Invalid vertex data: \"" + line + "\"");
		exit(EXIT_FAILURE);
	}

	return { std::stof(parts[0]), std::stof(parts[1]), std::stof(parts[2]) };
}

void ObjReader::readEntity(std::string& line)
{
	entities.push_back(std::make_unique<Entity>());
	Entity& entity = *entities.back();

	size_t start = line.find('(') + 1;
	size_t end = line.find(')');
	entity.classname = line.substr(start, end - start);
	std::getline(m_file, line);

	while (true)
	{
		if (line.starts_with('#'))
		{
			std::getline(m_file, line);
			continue;
		}

		if (line.starts_with(c_SMOOTH_PREFIX))
		{
			line.replace(0, c_SMOOTH_PREFIX.length(), "");
			std::getline(m_file, line);
			continue;
		}

		if (line.starts_with(c_GROUP_PREFIX))
		{
			readBrush(entity, line);
			continue;
		}

		break;
	}
}

void ObjReader::readBrush(M2PEntity::Entity& entity, std::string& line)
{
	entity.brushes.push_back(std::make_unique<Brush>());
	Brush& brush = *entity.brushes.back();
	std::getline(m_file, line);

	while (true)
	{
		if (line.starts_with('#'))
		{
			std::getline(m_file, line);
			continue;
		}

		if (line.starts_with(c_USEMTL_PREFIX))
		{
			line.replace(0, c_USEMTL_PREFIX.length(), "");
			readFace(brush, line);
			continue;
		}

		break;
	}
}

void ObjReader::readFace(M2PEntity::Brush &brush, std::string &line)
{
	std::string textureName = line;
	M2PWad3::ImageSize imageInfo = wadHandler.checkTexture(textureName);
	std::getline(m_file, line);

	while (true)
	{
		if (line.starts_with('#'))
		{
			std::getline(m_file, line);
			continue;
		}

		if (line.starts_with(c_FACE_PREFIX))
		{
			Face face;
			face.texture.name = textureName;

			line.replace(0, c_FACE_PREFIX.length(), "");
			std::vector<std::string> parts = M2PUtils::split(line, ' ');

			for (const std::string& part : parts)
			{
				std::vector<std::string> pointParts = M2PUtils::split(part, '/');
				int indexV = std::stoi(pointParts[0]) - 1;
				int indexT = std::stoi(pointParts[1]) - 1;
				int indexN = std::stoi(pointParts[2]) - 1;

				Vertex vertex{ m_vertexCoords[indexV] };
				vertex.uv = m_uvCoords[indexT];
				vertex.normal = m_normalCoords[indexN];

				face.vertices.push_back(vertex);
			}

			Vector3 planePoints[3] = { face.vertices[0].coord(), face.vertices[1].coord(), face.vertices[2].coord() };
			face.normal = M2PGeo::planeNormal(planePoints);

			brush.faces.push_back(face);

			std::getline(m_file, line);
			continue;
		}

		break;
	}
}
