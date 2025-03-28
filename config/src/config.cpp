#include <iostream>
#include "config.h"
#include "logging.h"


const char* NAME = { "Map2Prop++ v0.1.0" };


// Defaults
std::string Config::input;
std::string Config::output;
std::string Config::gameConfig;
std::string Config::studiomdl;
std::string Config::wadList;
std::string Config::outputname;
bool Config::autocompile = true;
bool Config::mapcompile = false;
bool Config::renameChrome = false;
int Config::wadCache = 10;
float Config::smoothing = 60.0f;
float Config::timeout = 60.0f;
float Config::qcScale = 1.0f;
float Config::qcGamma = 1.8f;
float Config::qcRotate = 0.0f;
Vector3D Config::qcOffset = { 0.0f, 0.0f, 0.0f };
std::filesystem::path Config::inputPath;


static void printVersionAndExit()
{
    std::cout << NAME << std::endl;
    exit(EXIT_SUCCESS);
}

static void printUsageAndExit()
{
    std::cout << "Usage: Map2Prop input [options]\n" <<
        "Converts a .map/.rmf/.jmf or J.A.C.K .obj into GoldSrc .smd files for model creation.\n\n" <<
        "Arguments:\n" <<
        "  input                 .map/.rmf/.jmf/.obj file to convert\n" <<
        "  -v | --version        print current version\n" <<
        "  -h | --help           show this help message and exit\n\n" <<
        "Options:\n" <<
        "  -a | --noautocompile  automatically compile the model after conversion\n" <<
        "  -c | --mapcompile     modify .map input to replace func_map2prop with model entities after compile\n" <<
        "  -o | --output         specify an output directory\n" <<
        "  -g | --gameconfig     game config to use from config.ini\n" <<
        "  -m | --studiomdl      path to SC studiomdl.exe\n" <<
        "  -w | --wadlist        path to text file listing .wad files\n" <<
        "  -n | --wadcache       max number of .wad files to keep in memory\n" <<
        "  -s | --smoothing      angle threshold for applying smoothing (use 0 to smooth all edges)\n" <<
        "  -t | --time           timeout for running studiomdl.exe (default 60.0 seconds)\n" <<
        "  --renamechrome        rename chrome textures (disables chrome)\n\n" <<
        "QC options:\n" <<
        "  --outputname          filename for the finished model\n" <<
        "  --scale               scale the model by this amount (default 1.0)\n" <<
        "  --gamma               darken/brighten textures (default 1.8)\n" <<
        "  --offset x y z        X Y Z offset to apply to the model (default 0 0 0)\n" <<
        "  --rotate              rotate the model by this many degrees" <<
        std::endl;
    exit(EXIT_SUCCESS);
}


void Config::handleArgs(int argc, char** argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0)
            printVersionAndExit();
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
            printUsageAndExit();

        if (strcmp(argv[i], "--noautocompile") == 0 || strcmp(argv[i], "-a") == 0)
        {
            Config::autocompile = false;
            continue;
        }
        if (strcmp(argv[i], "--mapcompile") == 0 || strcmp(argv[i], "-c") == 0)
        {
            Config::mapcompile = true;
            continue;
        }
        if (strcmp(argv[i], "--output") == 0 || strcmp(argv[i], "-o") == 0)
        {
            i++;
            if (i < argc)
                Config::output = argv[i];
            continue;
        }
        if (strcmp(argv[i], "--gameconfig") == 0 || strcmp(argv[i], "-g") == 0)
        {
            i++;
            if (i < argc)
                Config::gameConfig = argv[i];
            continue;
        }
        if (strcmp(argv[i], "--studiomdl") == 0 || strcmp(argv[i], "-m") == 0)
        {
            i++;
            if (i < argc)
                Config::studiomdl = argv[i];
            continue;
        }
        if (strcmp(argv[i], "--wadlist") == 0 || strcmp(argv[i], "-w") == 0)
        {
            i++;
            if (i < argc)
                Config::wadList = argv[i];
            continue;
        }
        if (strcmp(argv[i], "--wadcache") == 0 || strcmp(argv[i], "-n") == 0)
        {
            i++;
            if (i < argc)
                Config::wadCache = std::stoi(argv[i]);
            continue;
        }
        if (strcmp(argv[i], "--smoothing") == 0 || strcmp(argv[i], "-s") == 0)
        {
            i++;
            if (i < argc)
                Config::smoothing = std::stof(argv[i]);
            continue;
        }
        if (strcmp(argv[i], "--timeout") == 0 || strcmp(argv[i], "-t") == 0)
        {
            i++;
            if (i < argc)
                Config::timeout = std::stof(argv[i]);
            continue;
        }
        if (strcmp(argv[i], "--renamechrome") == 0)
        {
            Config::renameChrome = false;
            continue;
        }


        if (strncmp(argv[i], "-", 1) == 0)
        {
            Logging::error(std::string{ "Unknown argument: \"" } + argv[i] + "\"\n");
            exit(EXIT_FAILURE);
        }

        if (!Config::input.empty())
        {
            Logging::error(std::string{ "Unknown argument: \"" } + argv[i] + "\"\n");
            exit(EXIT_FAILURE);
        }
        Config::input = argv[i];
    }

    if (Config::input.empty())
    {
        printf("Missing positional argument: input (.map/.rmf/.jmf/.obj file to convert)");
        exit(EXIT_FAILURE);
    }
}
