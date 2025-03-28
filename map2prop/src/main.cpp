#include <iostream>
#include <filesystem>
#include "logging.h"
#include "config.h"


int main(int argc, char** argv)
{
    Logging::debug("Hello World!");
    Logging::info("Checking args...");

    Config::handleArgs(argc, argv);

    if (!Config::input.empty())
        std::cout << Config::input << std::endl;

    std::cout << Logging::LogLevelName[Logging::loglevel] << std::endl;
}
