#include <string>
#include <sstream>
#include "map_format.h"
#include "logging.h"
#include "utils.h"


static inline Logging::Logger& logger = Logging::Logger::getLogger("mapreader");

using namespace M2PMAP;
using namespace M2PFormat;
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
	m_file.close();
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
			entities.emplace_back(std::make_unique<Entity>());
			readEntity(*entities.back());
		}
	}
}


void MapReader::readEntity(Entity &entity)
{
	// Reseve .5M for worldspawn, 2048 otherwise
	entity.raw.reserve((entities.size() == 0) ? static_cast<size_t>(5e5) : 2048);
	entity.raw += "{\n";

	std::string line, key, value;
	line.reserve(512);
	while (std::getline(m_file, line))
	{
		entity.raw += line + "\n";

		if (line.starts_with("//"))
		{
			continue;
		}
		else if (line.starts_with('"'))
		{
			const std::vector<std::string>& parts = M2PUtils::split(line, '"');
			if (parts.size() != 4)
			{
				logger.error("Invalid entity property: \"" + line + "\"");
				exit(EXIT_FAILURE);
			}
			key = parts.at(1);
			value = parts.at(3);
			if (key == "classname")
				entity.classname = value;
			entity.keyvalues.emplace_back(key, value);
		}
		else if (line.starts_with('{'))
		{
			bool valid = true;

			entity.brushes.emplace_back(std::make_unique<Brush>());
			Brush& brush = *entity.brushes.back();
			readBrush(brush, line, valid);

			if (valid)
				entity.raw += brush.raw;
			else
				entity.brushes.pop_back();
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
}


void MapReader::readBrush(Brush& brush, std::string& line, bool &outValid)
{
	brush.raw.reserve(512);
	brush.raw = "";
	std::vector<Plane> planes;

	while (std::getline(m_file, line))
	{
		if (line.starts_with("//"))
			continue;

		if (line.starts_with('('))
		{
			if (!outValid)
				continue;

			brush.raw += line + "\n";

			const std::vector<std::string>& parts = M2PUtils::split(line, ' ');
			if (parts.size() != 31)
			{
				logger.error("Unexpected face data: \"" + line + "\"");
				exit(EXIT_FAILURE);
			}

			Vector3 planePoints[3] = {
				Vector3{ std::stof(parts[1]), std::stof(parts[2]), std::stof(parts[3]) },
				Vector3{ std::stof(parts[6]), std::stof(parts[7]), std::stof(parts[8]) },
				Vector3{ std::stof(parts[11]), std::stof(parts[12]), std::stof(parts[13]) }
			};
			std::string textureName = parts[15];


			M2PWad3::ImageSize imageInfo = wadHandler.checkTexture(textureName);

			Texture texture{
				.name = textureName,
				.shiftx = std::stof(parts[20]),
				.shifty = std::stof(parts[26]),
				.angle = std::stof(parts[28]),
				.scalex = std::stof(parts[29]),
				.scaley = std::stof(parts[30]),
				.width = imageInfo.width,
				.height = imageInfo.height,
				.rightaxis = { std::stof(parts[17]), std::stof(parts[18]), std::stof(parts[19]) },
				.downaxis = { std::stof(parts[23]), std::stof(parts[24]), std::stof(parts[25]) }
			};

			if (M2PGeo::segmentsCross(planePoints) == Vector3::zero())
			{
				logger.warning("Plane points may not form a line. Near " +
					std::format("({} {} {})", parts[1], parts[2], parts[3]));
				outValid = false;
				continue;
			}

			planes.emplace_back(planePoints, texture);
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

	if (outValid && !planes.empty())
		planesToFaces(planes, brush.faces);
}


bool M2PMAP::intersection3Planes(const HessianPlane& p1, const HessianPlane& p2, const HessianPlane& p3, Vector3& intersectionOut)
{
	Vector3 n1 = p1.normal(); Vector3 n2 = p2.normal(); Vector3 n3 = p3.normal();
	FP d1 = p1.distance(); FP d2 = p2.distance(); FP d3 = p3.distance();

	FP denominator = n1.dot(n2.cross(n3));
	if (abs(denominator) < c_EPSILON/100)
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
	for (const Plane& plane : planes)
	{
		if (plane.pointRelation(point) == PointRelation::INFRONT)
			return true;
	}
	return false;
}

static inline void addPointUnique(std::vector<Vertex>& vertices, const Vector3 &point)
{
	for (const auto& vertex : vertices)
	{
		if (vertex == point)
			return;
	}
	vertices.emplace_back(point);
}

void M2PMAP::planesToFaces(const std::vector<Plane>& planes, std::vector<Face> &facesOut)
{
	size_t numPlanes = planes.size();
	facesOut.assign(numPlanes, {});

	for (int i = 0; i < numPlanes - 2; ++i)
	{
		for (int j = i; j < numPlanes - 1; ++j)
		{
			for (int k = j; k < numPlanes; ++k)
			{
				if (i == j && i == k)
					continue;

				Vector3 intersection;

				if (!intersection3Planes(planes[i], planes[j], planes[k], intersection))
					continue;

				if (isPointOutsidePlanes(planes, intersection))
					continue;

				addPointUnique(facesOut[i].vertices, intersection);
				addPointUnique(facesOut[j].vertices, intersection);
				addPointUnique(facesOut[k].vertices, intersection);

				facesOut[i].texture = planes[i].texture();
				facesOut[j].texture = planes[j].texture();
				facesOut[k].texture = planes[k].texture();

				facesOut[i].normal = planes[i].normal();
				facesOut[j].normal = planes[j].normal();
				facesOut[k].normal = planes[k].normal();
			}
		}
	}

	if (std::erase_if(facesOut, [](Face face) { return face.vertices.size() < 3; }))
		logger.warning("Faces with fewer than 3 vertices skipped");

	for (Face& face : facesOut)
	{
		sortVertices(face.vertices, face.normal);
		for (M2PGeo::Vertex &vertex : face.vertices)
		{
			vertex.uv = face.texture.uvForPoint(vertex);
			vertex.normal = face.normal;
		}
	}
}
