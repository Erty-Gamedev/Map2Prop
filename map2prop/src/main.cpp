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
        M2PMap::MapReader mapReader = M2PMap::MapReader(g_config.inputFilepath, g_config.outputDir);

        auto models = M2PExport::prepareModels(mapReader.entities, mapReader.wadHandler);
        int res = M2PExport::processModels(models, mapReader.hasMissingTextures());

        if (g_config.mapcompile && !res)
            M2PExport::rewriteMap(mapReader.entities);
    }
    catch (const std::exception& e)
    {
        logger.error(e.what());
    }
}
