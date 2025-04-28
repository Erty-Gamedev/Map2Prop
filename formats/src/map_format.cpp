#include <string>
#include <sstream>
#include "map_format.h"
#include "config.h"
#include "logging.h"
#include "utils.h"


static inline Logging::Logger& logger = Logging::Logger::getLogger("mapreader");

using namespace M2PMap;
using namespace M2PGeo;
using namespace M2PEntity;

MapReader::MapReader(const std::filesystem::path& filepath, const std::filesystem::path& outputDir)
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
MapReader::~MapReader()
{
	if (m_file.is_open())
		m_file.close();
}

void MapReader::parse()
{
	std::string line;
	line.reserve(1024);
	while (std::getline(m_file, line))
	{
		if (line.starts_with('{'))
		{
			Entity entity = readEntity();
			m_entities.push_back(entity);
		}
	}
}


Entity MapReader::readEntity()
{
	Entity entity;
	entity.raw += "{\n";

	std::string line, key, value;
	line.reserve(1024);
	while (std::getline(m_file, line))
	{
		entity.raw += line;

		if (line.starts_with("//"))
		{
			continue;
		}
		else if (line.starts_with('"'))
		{
			std::vector<std::string> parts = M2PUtils::split(line, '"');
			if (parts.size() > 5)
			{
				logger.error("Invalid entity property: \"" + line + "\"");
				exit(EXIT_FAILURE);
			}
			key = parts.at(1);
			value = parts.at(3);
			if (key == "classname")
				entity.classname = value;
			entity.properties[key] = value;
		}
		else if (line.starts_with('{'))
		{
			Brush brush = readBrush();
			entity.brushes.push_back(brush);
		}
		else if (line.starts_with('}'))
		{
			break;
		}
		else
		{
			logger.error("Unexpected entity data: \"" + line + "\"");
			exit(EXIT_FAILURE);
		}
	}

	return entity;
}


bool M2PMap::intersection3Planes(const HessianPlane& p1, const HessianPlane& p2, const HessianPlane& p3, Vector3& intersectionOut)
{
	Vector3 n1 = p1.normal(); Vector3 n2 = p2.normal(); Vector3 n3 = p3.normal();
	float d1 = p1.distance(); float d2 = p2.distance(); float d3 = p3.distance();

	float denominator = n1.dot(n2.cross(n3));
	if (abs(denominator) < c_EPSILON)
		return false;

	intersectionOut = -(
		-d1 * n2.cross(n3)
		- d2 * n3.cross(n1)
		- d3 * n1.cross(n2)
	) / denominator;

	return true;
}

static bool isPointOutsidePlanes(const std::vector<Plane>& planes, const Vector3& point)
{
	for (Plane plane : planes)
	{
		if (plane.pointRelation(point) > PointRelation::INFRONT)
			return true;
	}
	return false;
}

std::vector<Face> M2PMap::planesToFaces(const std::vector<Plane>& planes)
{
	size_t numPlanes = planes.size();
	std::vector<Face> faces(numPlanes, {});

	for (int i = 0; i < numPlanes - 2; ++i)
	{
		for (int j = i; j < numPlanes - 1; ++j)
		{
			for (int k = j; k < numPlanes - 0; ++k)
			{
				if (i == j && i == k)
					continue;

				Vector3 intersection;

				if (!intersection3Planes(planes[i], planes[j], planes[k], intersection))
					continue;

				if (isPointOutsidePlanes(planes, intersection))
					continue;

				faces[i].points.push_back(intersection);
				faces[j].points.push_back(intersection);
				faces[k].points.push_back(intersection);

				faces[i].texture = planes[i].texture();
				faces[j].texture = planes[j].texture();
				faces[k].texture = planes[k].texture();

				faces[i].normal = planes[i].normal();
				faces[j].normal = planes[j].normal();
				faces[k].normal = planes[k].normal();
			}
		}
	}

	for (Face& face : faces)
	{
		sortVectors(face.points, face.normal);
		for (auto const& point : face.points)
		{
			Vector2 uv = face.texture.uvForPoint(point);
			face.vertices.emplace_back(point, face.normal, uv, false);
		}

	}

	return faces;
}


Brush MapReader::readBrush()
{
	Brush brush;
	brush.raw = "";
	std::vector<Plane> planes;

	std::string line;
	line.reserve(1024);
	while (std::getline(m_file, line))
	{
		if (line.starts_with("//"))
			continue;
		brush.raw += line;

		if (line.starts_with('('))
		{
			std::vector<std::string> parts = M2PUtils::split(line, ' ');
			if (parts.size() != 31)
			{
				logger.error("Unexpected face data: \"" + line + "\"");
				exit(EXIT_FAILURE);
			}

			Vector3 plane_points[3] = {
				Vector3{ std::stof(parts[1]), std::stof(parts[2]), std::stof(parts[3]) },
				Vector3{ std::stof(parts[6]), std::stof(parts[7]), std::stof(parts[8]) },
				Vector3{ std::stof(parts[11]), std::stof(parts[12]), std::stof(parts[13]) }
			};
			std::string textureName = parts[15];


			M2PWad3::ImageSize imageInfo = m_wadHandler.checkTexture(textureName);

			Texture texture{
				.name = textureName,
				.shiftx = std::stof(parts[20]),
				.shifty = std::stof(parts[26]),
				.angle = std::stof(parts[28]),
				.scalex = std::stof(parts[29]),
				.scaley = std::stof(parts[30]),
				.width = (float)imageInfo.width,
				.height = (float)imageInfo.height,
				.rightaxis = { std::stof(parts[17]), std::stof(parts[18]), std::stof(parts[19]) },
				.downaxis = { std::stof(parts[23]), std::stof(parts[24]), std::stof(parts[25]) }
			};

			planes.emplace_back(plane_points, texture);
		}
		else if (line.starts_with('}'))
		{
			break;
		}
		else
		{
			logger.error("Unexpected face data: \"" + line + "\"");
			exit(EXIT_FAILURE);
		}
	}

	brush.faces = planesToFaces(planes);

	return brush;
}

bool MapReader::hasMissingTextures() const { return m_wadHandler.hasMissingTextures(); }
std::vector<Entity>& MapReader::getEntities()
{
	return m_entities;
}
