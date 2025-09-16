#include "logging.h"
#include "config.h"
#include "geometry.h"
#include "map_format.h"
#include "rmf_format.h"
#include "jmf_format.h"
#include "obj_format.h"
#include "ol_format.h"
#include "export.h"


static Logging::Logger& logger = Logging::Logger::getLogger("map2prop");


int main(int argc, char** argv)
{
    M2PConfig::handleArgs(argc, argv);
    using M2PConfig::g_config;

    if (g_config.eager)
    {
        M2PGeo::g_isEager = true;
        logger.debug("Using eager algorithm");
    }

    try
    {
        M2PEntity::BaseReader reader;

        switch (g_config.extension)
        {
        case M2PConfig::Extension::MAP:
            reader = M2PFormat::MapReader(g_config.inputFilepath, g_config.outputDir);
            break;
        case M2PConfig::Extension::RMF:
            reader = M2PFormat::RmfReader(g_config.inputFilepath, g_config.outputDir);
            break;
        case M2PConfig::Extension::JMF:
            reader = M2PFormat::JmfReader(g_config.inputFilepath, g_config.outputDir);
            break;
        case M2PConfig::Extension::OBJ:
            reader = M2PFormat::ObjReader(g_config.inputFilepath, g_config.outputDir);
            break;
        case M2PConfig::Extension::OL:
            M2PFormat::OlReader olReader = M2PFormat::OlReader(g_config.inputFilepath, g_config.outputDir);
            return olReader.process();
        }

        std::unordered_map<std::string, M2PExport::ModelData> models = M2PExport::prepareModels(reader);

        if (models.empty())
        {
            if (reader.entities[0]->getKey(M2PExport::c_NOTE_KEY) == M2PExport::c_NOTE_VALUE)
                logger.info(g_config.input + " was already converted and had no new models to convert");
            else
                logger.info(g_config.input + " had no models to convert");

            return EXIT_SUCCESS;
        }


        std::vector<std::filesystem::path> successes;
        successes.reserve(models.size());

        int res = M2PExport::processModels(models, reader.hasMissingTextures(), successes);


        if (res > 0)
            logger.warning("Something went wrong during compilation. Check logs for more info");

        size_t numSuccesses = successes.size();
        if (numSuccesses != 0)
        {
            std::sort(successes.begin(), successes.end());
            logger.log("\n");
            logger.info(std::format("Finished compiling {} model{}:", numSuccesses, numSuccesses == 1 ? "" : "s"));

            std::string successList{ "" };
            for (const std::filesystem::path& successPath : successes)
                successList += Styling::style(Styling::success)
                + std::filesystem::absolute(g_config.extractDir() / successPath).string() + Styling::style() + "\n";
            logger.log(successList);
        }

        if (g_config.mapcompile && !res)
            M2PExport::rewriteMap(reader.entities);

        return res;
    }
    catch (const std::exception& e)
    {
        logger.error(e.what());
        return EXIT_FAILURE;
    }
}
