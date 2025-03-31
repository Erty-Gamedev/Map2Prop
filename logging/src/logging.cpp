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


void Logging::log(Logging::LogLevel level, const std::string& message)
{
    if (Logging::loglevel > level)
        return;

    const char * levelName = Logging::LogLevelName[level];

    if (!isVirtual)
    {
        printf("%1s: %2s\n", levelName, message.c_str());
        return;
    }

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
        printf("%1s: %2s%3s\n", levelName, message.c_str(), "\033[0m");
        return;
    }

    printf("%1s%2s: %3s%4s\n", styling.c_str(), levelName, message.c_str(), "\033[0m");
}

void Logging::debug(const std::string& message)
{
    Logging::log(Logging::LogLevel::LOG_DEBUG, message);
}
void Logging::info(const std::string& message)
{
    Logging::log(Logging::LogLevel::LOG_INFO, message);
}
void Logging::warning(const std::string& message)
{
    Logging::log(Logging::LogLevel::LOG_WARNING, message);
}
void Logging::error(const std::string& message)
{
    Logging::log(Logging::LogLevel::LOG_ERROR, message);
}
