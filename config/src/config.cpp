#include <iostream>
#include <fstream>
#include <format>
#include "config.h"
#include "logging.h"
#include "utils.h"


const char* NAME = { "Map2Prop++ v0.1.0" };

const char* USAGE = R"USAGE(Usage: Map2Prop input [options]
Converts a .map/.rmf/.jmf or J.A.C.K .obj into GoldSrc .smd files for model creation.

Arguments:
  input                 .map/.rmf/.jmf/.obj file to convert
  -v | --version        print current version
  -h | --help           show this help message and exit

Options:
  -c | --mapcompile     modify .map input to replace func_map2prop with model entities after compile
  -a | --noautocompile  do not automatically compile the model after conversion
  -o | --output         specify an output directory
  -g | --gameconfig     game config to use from config.ini
  -m | --studiomdl      path to SC studiomdl.exe
  -w | --wadlist        path to text file listing .wad files
  -n | --wadcache       max number of .wad files to keep in memory
  -s | --smoothing      angle threshold for applying smoothing (use 0 to smooth all edges)
  -t | --time           timeout for running studiomdl.exe (default 60.0 seconds)
  --renamechrome        rename chrome textures (disables chrome)

QC options:
  --outputname          filename for the finished model
  --scale               scale the model by this amount (default 1.0)
  --gamma               darken/brighten textures (default 1.8)
  --offset x y z        X Y Z offset to apply to the model (default 0 0 0)
  --rotate              rotate the model by this many degrees
)USAGE";


using M2PConfig::config;
M2PConfig::Config config{};


static Logging::Logger& logger = Logging::Logger::getLogger("map2prop");


static inline void loadFromFileConfig(const M2PConfig::ConfigFile& configFile)
{
    std::string value;
    if (!(value = configFile.getConfig("output directory")).empty())
        config.outputDir = value;

    if (!(value = configFile.getConfig("steam directory")).empty())
        config.steamDir = value;

    if (!(value = configFile.getConfig("game config")).empty())
        config.gameConfig = value;

    if (!(value = configFile.getConfig("studiomdl")).empty())
        config.studiomdl = value;

    if (!(value = configFile.getConfig("autocompile")).empty())
        config.autocompile = M2PUtils::strToBool(value);

    if (!(value = configFile.getConfig("timeout")).empty())
        config.timeout = std::stof(value);

    if (!(value = configFile.getConfig("wad cache")).empty())
        config.wadCache = std::stoi(value);
}


void M2PConfig::handleArgs(int argc, char** argv)
{
    M2PConfig::ConfigFile configFile{};
    loadFromFileConfig(configFile);

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0)
        {
            logger.log(NAME);
            exit(EXIT_SUCCESS);
        }
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
        {
            logger.log(USAGE);
            exit(EXIT_SUCCESS);
        }

        if (strcmp(argv[i], "--noautocompile") == 0 || strcmp(argv[i], "-a") == 0)
        {
            config.autocompile = false;
            continue;
        }
        if (strcmp(argv[i], "--mapcompile") == 0 || strcmp(argv[i], "-c") == 0)
        {
            config.mapcompile = true;
            continue;
        }
        if (strcmp(argv[i], "--output") == 0 || strcmp(argv[i], "-o") == 0)
        {
            i++;
            if (i < argc)
                config.output = argv[i];
            continue;
        }
        if (strcmp(argv[i], "--gameconfig") == 0 || strcmp(argv[i], "-g") == 0)
        {
            i++;
            if (i < argc)
                config.gameConfig = argv[i];
            continue;
        }
        if (strcmp(argv[i], "--studiomdl") == 0 || strcmp(argv[i], "-m") == 0)
        {
            i++;
            if (i < argc)
                config.studiomdl = argv[i];
            continue;
        }
        if (strcmp(argv[i], "--wadlist") == 0 || strcmp(argv[i], "-w") == 0)
        {
            i++;
            if (i < argc)
            {
                std::filesystem::path wadListFilepath = argv[i];
                if (!std::filesystem::exists(wadListFilepath))
                {
                    logger.error("Could not find file \"" + wadListFilepath.string() + "\"");
                    exit(EXIT_FAILURE);
                }

                std::ifstream wadListFile;
                wadListFile.open(wadListFilepath);
                if (!wadListFile.is_open() || !wadListFile.good())
                {
                    wadListFile.close();
                    logger.error("Could not open file \"" + wadListFilepath.string() + "\"");
                    exit(EXIT_FAILURE);
                }

                config.wadList.reserve(16);
                std::string line;
                line.reserve(256);
                while (std::getline(wadListFile, line))
                {
                    config.wadList.emplace_back(line);
                }

                wadListFile.close();
            }
            continue;
        }
        if (strcmp(argv[i], "--wadcache") == 0 || strcmp(argv[i], "-n") == 0)
        {
            i++;
            if (i < argc)
                config.wadCache = std::stoi(argv[i]);
            continue;
        }
        if (strcmp(argv[i], "--smoothing") == 0 || strcmp(argv[i], "-s") == 0)
        {
            i++;
            if (i < argc)
                config.smoothing = std::stof(argv[i]);
            continue;
        }
        if (strcmp(argv[i], "--timeout") == 0 || strcmp(argv[i], "-t") == 0)
        {
            i++;
            if (i < argc)
                config.timeout = std::stof(argv[i]);
            continue;
        }
        if (strcmp(argv[i], "--renamechrome") == 0)
        {
            config.renameChrome = false;
            continue;
        }


        if (strncmp(argv[i], "-", 1) == 0)
        {
            logger.error(std::string{ "Unknown argument: \"" } + argv[i] + "\"\n");
            exit(EXIT_FAILURE);
        }

        if (!config.input.empty())
        {
            logger.error(std::string{ "Unknown argument: \"" } + argv[i] + "\"\n");
            exit(EXIT_FAILURE);
        }
        config.input = argv[i];
    }

    if (config.input.empty())
    {
        logger.log("Missing positional argument: input (.map/.rmf/.jmf/.obj file to convert)");
        logger.log(USAGE);
        exit(EXIT_FAILURE);
    }

    if (!std::filesystem::exists(config.input))
    {
        logger.error("Could not open file \"" + config.input + "\"");
        exit(EXIT_FAILURE);
    }

    config.inputFilepath = config.input;
    config.inputDir = config.inputFilepath.parent_path();

    if (!config.output.empty())
        config.outputDir = config.output;
    else
        config.outputDir = config.inputDir;

    configFile.setGameConfig(config.gameConfig);
    M2PUtils::extendVector(config.wadList, configFile.getWadList());

    std::string value;
    if (!(value = configFile.getConfig("game")).empty())
        config.game = value;
    if (!(value = configFile.getConfig("mod")).empty())
        config.mod = value;

    logger.debug("Configs loaded");
}
