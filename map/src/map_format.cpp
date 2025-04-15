#include <filesystem>
#include <string>
#include <sstream>
#include "map_format.h"
#include "config.h"
#include "logging.h"


static inline Logging::Logger& logger = Logging::Logger::getLogger("mapreader");

using namespace M2PMap;

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




static std::vector<std::string> split(std::string& str, const char delimiter = ' ')
{
	std::istringstream strStream{ str };
	std::vector<std::string> segments;
	std::string segment;
	while (std::getline(strStream, segment, delimiter))
	{
		segments.push_back(segment);
	}
	return segments;
}


Entity MapReader::readEntity()
{
	Entity entity;
	entity.raw += "{\n";

	std::string key, value;

	std::string line;
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
			std::vector<std::string> parts = split(line, '"');
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
			std::vector<std::string> parts = split(line, ' ');
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

			if (!m_wadHandler.checkTexture(textureName))
				m_missingTextures = true;

			int width, height;
			if (m_wadHandler.isSkipTexture(textureName) || m_wadHandler.isToolTexture(textureName))
			{
				width = 16;
				height = 16;
			}
			else
			{
				M2PWad3::ImageInfo imageInfo{ textureName };
				width = imageInfo.width;
				height = imageInfo.height;
			}

			Texture texture{
				.name = textureName,
				.shiftx = std::stof(parts[20]),
				.shifty = std::stof(parts[26]),
				.angle = std::stof(parts[28]),
				.scalex = std::stof(parts[29]),
				.scaley = std::stof(parts[30]),
				.width = (float)width,
				.height = (float)height,
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
	return brush;
}

bool MapReader::hasMissingTextures() const { return m_missingTextures; }
