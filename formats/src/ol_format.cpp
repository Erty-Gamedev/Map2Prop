#include <string>
#include <format>
#include "ol_format.h"
#include "rmf_format.h"
#include "logging.h"
#include "config.h"
#include "utils.h"
#include "binutils.h"
#include "export.h"

static inline Logging::Logger& logger = Logging::Logger::getLogger("olreader");

using M2PConfig::g_config;

using namespace M2POL;
using namespace M2PFormat;
using namespace M2PGeo;
using namespace M2PEntity;
using namespace M2PBinUtils;


OlReader::OlReader(const std::filesystem::path& filepath, const std::filesystem::path& outputDir)
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
OlReader::~OlReader()
{
	if (m_file.is_open())
		m_file.close();
}

void OlReader::parse()
{
	PrefabLibHeader header{};
	m_file.read(reinterpret_cast<char*>(&header), sizeof(PrefabLibHeader));

	if (abs(header.version - 0.1) > 0.01)
	{
		logger.error(m_filepath.string() + " has unexpected version");
		exit(EXIT_FAILURE);
	}

	logger.info(std::format("Reading prefab library with {} prefabs", header.numEntries));

	m_entries.reserve(header.numEntries);
	for (unsigned int i = 0; i < header.numEntries; ++i)
	{
		PrefabHeader prefabHeader{};
		m_file.seekg(header.dirOffset + (i * sizeof(PrefabHeader)), std::ios::beg);

		m_file.read(reinterpret_cast<char*>(&prefabHeader), sizeof(PrefabHeader));
		m_entries.push_back(prefabHeader);
	}
}

int OlReader::process()
{
	int res = 0;

	std::vector<std::filesystem::path> successes;
	successes.reserve(m_entries.size() * 10);

	for (const PrefabHeader& entry : m_entries)
	{
		std::string filename = M2PUtils::slugify(entry.name);
		RmfReader reader = RmfReader(m_filepath, m_outputDir, entry.offset);

		std::unordered_map<std::string, M2PExport::ModelData> models = M2PExport::prepareModels(reader, filename);

		if (models.empty())
		{
			logger.info("Prefab " + filename + " had no models to convert, skipping");
			continue;
		}

		res += M2PExport::processModels(models, reader.hasMissingTextures(), successes);
	}

	if (res > 0)
		logger.warning("Something went wrong during compilation. Check logs for more info");

	size_t numSuccesses = successes.size();
	if (numSuccesses != 0)
	{
		std::sort(successes.begin(), successes.end());
		logger.log("\n");
		logger.info("Finished compiling %u model%c", numSuccesses, numSuccesses == 1 ? '\0' : 's');

		std::string successList{ "" };
		for (const std::filesystem::path& successPath : successes)
			successList += Styling::style(Styling::success)
			+ std::filesystem::absolute(g_config.extractDir() / successPath).string() + Styling::style() + "\n";
		logger.log(successList);
	}

	return res;
}
