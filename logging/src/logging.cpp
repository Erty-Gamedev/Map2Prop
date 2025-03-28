#include "logging.h"
#include <wchar.h>
#include <windows.h>


#ifdef _DEBUG
Logging::LogLevel Logging::loglevel = Logging::LogLevel::LOG_DEBUG;
#else
Logging::LogLevel Logging::loglevel = Logging::LogLevel::LOG_INFO;
#endif


/**
 * Check if we can enable virtual terminal (needed for ANSI escape sequences)
 * From: https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#example-of-enabling-virtual-terminal-processing
 */
static bool enableVirtualTerminal()
{
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD dwOriginalOutMode = 0;
    DWORD dwOriginalInMode = 0;
    if (!GetConsoleMode(hOut, &dwOriginalOutMode))
    {
        return false;
    }
    if (!GetConsoleMode(hIn, &dwOriginalInMode))
    {
        return false;
    }

    DWORD dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
    DWORD dwRequestedInModes = ENABLE_VIRTUAL_TERMINAL_INPUT;

    DWORD dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
    if (!SetConsoleMode(hOut, dwOutMode))
    {
        // We failed to set both modes, try to step down mode gracefully.
        dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
        if (!SetConsoleMode(hOut, dwOutMode))
        {
            // Failed to set any VT mode, can't do anything here.
            return false;
        }
    }

    DWORD dwInMode = dwOriginalInMode | dwRequestedInModes;
    if (!SetConsoleMode(hIn, dwInMode))
    {
        // Failed to set VT input mode, can't do anything here.
        return false;
    }

    return true;
}

static bool isVirtual = enableVirtualTerminal();


void Logging::log(const std::string& message, Logging::LogLevel level)
{
    if (Logging::loglevel > level)
        return;

    std::string levelName = Logging::LogLevelName[level];

    if (!isVirtual)
    {
        printf("%1s: %2s\n", levelName.c_str(), message.c_str());
        return;
    }

    //std::string styling = "\033[";
    std::string styling;
    styling.reserve(15);

    switch (level)
    {
    case Logging::LogLevel::LOG_INFO:
        styling = "\033[36m";
        break;
    case Logging::LogLevel::LOG_WARNING:
        styling = "\033[1;33m";
        break;
    case Logging::LogLevel::LOG_ERROR:
        styling = "\033[1;31m";
        break;
    default:
        printf("%1s: %2s%3s\n", levelName.c_str(), message.c_str(), "\033[0m");
        return;
    }

    printf("%1s%2s: %3s%4s\n", styling.c_str(), levelName.c_str(), message.c_str(), "\033[0m");
}

void Logging::debug(const std::string& message)
{
    Logging::log(message, Logging::LogLevel::LOG_DEBUG);
}
void Logging::info(const std::string& message)
{
    Logging::log(message, Logging::LogLevel::LOG_INFO);
}
void Logging::warning(const std::string& message)
{
    Logging::log(message, Logging::LogLevel::LOG_WARNING);
}
void Logging::error(const std::string& message)
{
    Logging::log(message, Logging::LogLevel::LOG_ERROR);
}
