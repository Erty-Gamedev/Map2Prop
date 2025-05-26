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
  --eager               use eager triangulation algorithm (faster)

QC options:
  --outputname          filename for the finished model
  --scale               scale the model by this amount (default 1.0)
  --gamma               darken/brighten textures (default 1.8)
  --offset x y z        X Y Z offset to apply to the model (default 0 0 0)
  --rotate              rotate the model by this many degrees
)USAGE";


using M2PConfig::g_config;
M2PConfig::Config g_config{};


static Logging::Logger& logger = Logging::Logger::getLogger("config");


static inline void loadFromFileConfig(const M2PConfig::ConfigFile& configFile)
{
    std::string value;
    if (!(value = configFile.getConfig("output directory")).empty())
        g_config.outputDir = value;

    if (!(value = configFile.getConfig("steam directory")).empty())
        g_config.steamDir = value;

    if (!(value = configFile.getConfig("game config")).empty())
        g_config.gameConfig = value;

    if (!(value = configFile.getConfig("studiomdl")).empty())
        g_config.studiomdl = value;

    if (!(value = configFile.getConfig("autocompile")).empty())
        g_config.autocompile = M2PUtils::strToBool(value);

    if (!(value = configFile.getConfig("timeout")).empty())
        g_config.timeout = std::stof(value);

    if (!(value = configFile.getConfig("wad cache")).empty())
        g_config.wadCache = std::stoi(value);
}


static inline void findWadsInDir(const std::filesystem::path& dir)
{
    std::filesystem::path filepath;
    std::string filestem;

    for (auto const& dirEntry : std::filesystem::directory_iterator{ dir })
    {
        filepath = dirEntry.path();
        if (M2PUtils::toLowerCase(filepath.extension().string()) != ".wad")
            continue;
        
        filestem = M2PUtils::toLowerCase(filepath.stem().string());
        if (M2PUtils::contains(M2PUtils::c_WADSKIPLIST, filestem))
            continue;

        if (M2PUtils::contains(g_config.wadList, filepath))
            continue;

        g_config.wadList.push_back(filepath);
    }
}


static inline std::string unSteamPipe(std::string str)
{
    for (auto const& steampipe : M2PUtils::c_STEAMPIPES)
    {
        size_t matchPosition = str.rfind(steampipe);
        if (matchPosition != std::string::npos)
        {
            str.replace(matchPosition, steampipe.length(), "");
            return str;
        }
    }
    return str;
}


static inline void populateWadList(M2PConfig::ConfigFile& configFile)
{
    M2PUtils::extendVectorUnique(g_config.wadList, configFile.getWadList());

    // Find WADs in input directory and output directory
    findWadsInDir(g_config.inputDir);
    findWadsInDir(g_config.outputDir);

    // Find WADs in mod directory
    std::filesystem::path modDir;
    std::filesystem::path gameDir = M2PConfig::gameDir();
    std::string mod = unSteamPipe(g_config.mod);

    modDir = { M2PConfig::gameDir() / mod };
    if (std::filesystem::is_directory(modDir))
        findWadsInDir(modDir);

    // Check steampipes
    for (const std::string& pipe : M2PUtils::c_STEAMPIPES)
    {
        modDir = { M2PConfig::gameDir() / (mod + pipe) };
        if (std::filesystem::is_directory(modDir))
            findWadsInDir(modDir);
    }
}


void M2PConfig::handleArgs(int argc, char** argv)
{
    // Check eager args
    for (int i = 1; i < argc; ++i)
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
    }

    g_config.wadList.reserve(128);

    M2PConfig::ConfigFile configFile{};
    loadFromFileConfig(configFile);

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--noautocompile") == 0 || strcmp(argv[i], "-a") == 0)
        {
            g_config.autocompile = false;
            continue;
        }
        if (strcmp(argv[i], "--mapcompile") == 0 || strcmp(argv[i], "-c") == 0)
        {
            g_config.mapcompile = true;
            continue;
        }
        if (strcmp(argv[i], "--output") == 0 || strcmp(argv[i], "-o") == 0)
        {
            ++i;
            if (i < argc)
                g_config.output = argv[i];
            continue;
        }
        if (strcmp(argv[i], "--gameconfig") == 0 || strcmp(argv[i], "-g") == 0)
        {
            ++i;
            if (i < argc)
                g_config.gameConfig = argv[i];
            continue;
        }
        if (strcmp(argv[i], "--studiomdl") == 0 || strcmp(argv[i], "-m") == 0)
        {
            ++i;
            if (i < argc)
                g_config.studiomdl = argv[i];
            continue;
        }
        if (strcmp(argv[i], "--wadlist") == 0 || strcmp(argv[i], "-w") == 0)
        {
            ++i;
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

                g_config.wadList.reserve(16);
                std::string line;
                line.reserve(256);
                while (std::getline(wadListFile, line))
                {
                    g_config.wadList.emplace_back(line);
                }

                wadListFile.close();
            }
            continue;
        }
        if (strcmp(argv[i], "--wadcache") == 0 || strcmp(argv[i], "-n") == 0)
        {
            ++i;
            if (i < argc)
                g_config.wadCache = std::stoi(argv[i]);
            continue;
        }
        if (strcmp(argv[i], "--smoothing") == 0 || strcmp(argv[i], "-s") == 0)
        {
            ++i;
            if (i < argc)
                g_config.smoothing = std::stof(argv[i]);
            continue;
        }
        if (strcmp(argv[i], "--timeout") == 0 || strcmp(argv[i], "-t") == 0)
        {
            ++i;
            if (i < argc)
                g_config.timeout = std::stof(argv[i]);
            continue;
        }
        if (strcmp(argv[i], "--renamechrome") == 0)
        {
            g_config.renameChrome = true;
            continue;
        }
        if (strcmp(argv[i], "--eager") == 0)
        {
            g_config.eager = true;
            continue;
        }


        if (strncmp(argv[i], "-", 1) == 0)
        {
            logger.error(std::string{ "Unknown argument: \"" } + argv[i] + "\"\n");
            exit(EXIT_FAILURE);
        }

        if (!g_config.input.empty())
        {
            logger.error(std::string{ "Unknown argument: \"" } + argv[i] + "\"\n");
            exit(EXIT_FAILURE);
        }
        g_config.input = argv[i];
    }

    if (g_config.input.empty())
    {
        logger.log("Missing positional argument: input (.map/.rmf/.jmf/.obj file to convert)");
        logger.log(USAGE);
        exit(EXIT_FAILURE);
    }

    if (!std::filesystem::exists(g_config.input))
    {
        logger.error("Could not open file \"" + g_config.input + "\"");
        exit(EXIT_FAILURE);
    }

    g_config.inputFilepath = g_config.input;
    g_config.inputDir = g_config.inputFilepath.parent_path();

    logger.info(g_config.input);

    if (!g_config.output.empty())
        g_config.outputDir = g_config.output;
    else
        g_config.outputDir = g_config.inputDir;

    configFile.setGameConfig(g_config.gameConfig);

    std::string value;
    if (!(value = configFile.getConfig("game")).empty())
        g_config.game = value;
    if (!(value = configFile.getConfig("mod")).empty())
        g_config.mod = value;

    populateWadList(configFile);

    logger.debug("Configs loaded");

    if (g_config.mapcompile)
        logger.info("Using --mapcompile");
}

std::filesystem::path M2PConfig::gameDir()
{
    if (g_config.steamDir.empty() || g_config.game.empty())
        return "";
    return g_config.steamDir / "steamapps" / "common" / g_config.game;
}
std::filesystem::path M2PConfig::modDir()
{
    if (gameDir().empty() || g_config.mod.empty())
        return "";
    return gameDir() / g_config.mod;
}

bool M2PConfig::isMap()
{
    return M2PUtils::toLowerCase(g_config.inputFilepath.extension().string()) == ".map";
}
bool M2PConfig::isObj()
{
    return M2PUtils::toLowerCase(g_config.inputFilepath.extension().string()) == ".obj";
}
