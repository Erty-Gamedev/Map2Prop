#include "logging.h"
#include "config.h"
#include "geometry.h"
#include "map_format.h"
#include "export.h"


using Logging::Logger;
static Logger& logger = Logger::getLogger("map2prop");


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
            reader = M2PMap::MapReader(g_config.inputFilepath, g_config.outputDir);
            break;
        case M2PConfig::Extension::RMF:
            logger.error("RMF not implemented");
            break;
        case M2PConfig::Extension::JMF:
            logger.error("JMF not implemented");
            break;
        case M2PConfig::Extension::OBJ:
            logger.error("OBJ not implemented");
            break;
        case M2PConfig::Extension::OL:
            logger.error("OL not implemented");
            break;
        }

        std::vector<M2PExport::ModelData> models = M2PExport::prepareModels(reader);

        if (models.empty())
        {
            if (reader.entities[0].getKey(M2PExport::c_NOTE_KEY) == M2PExport::c_NOTE_VALUE)
                logger.info(g_config.input + " was already converted and had no new models to convert");
            else
                logger.info(g_config.input + " had no models to convert");

            return EXIT_SUCCESS;
        }

        int res = M2PExport::processModels(models, reader.hasMissingTextures());

        if (g_config.mapcompile && !res)
            M2PExport::rewriteMap(reader.entities);
    }
    catch (const std::exception& e)
    {
        logger.error(e.what());
    }
}
