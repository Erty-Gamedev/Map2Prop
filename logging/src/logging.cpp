#include "logging.h"
#include <wchar.h>
#include <windows.h>


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


using namespace Logging;


Logger::Logger(const std::string& name)
{
    m_name = name;
    m_loglevel = DEFAULT_LOG_LEVEL;
}
void Logger::_log(const LogLevel& level, const char* message)
{
    if (level < m_loglevel) { return; }
    m_consoleHandler.log(level, message);
    m_fileHandler.log(level, message);
}
void Logger::debug(const char* message) { _log(LogLevel::LOG_DEBUG, message); }
void Logger::log(const char* message) { _log(LogLevel::LOG_LOG, message); }
void Logger::info(const char* message) { _log(LogLevel::LOG_INFO, message); }
void Logger::warning(const char* message) { _log(LogLevel::LOG_WARNING, message); }
void Logger::warn(const char* message) { Logger::warning(message); }
void Logger::error(const char* message) { _log(LogLevel::LOG_ERROR, message); }

std::string Logger::getName() const { return m_name; }

void Logger::setLevel(const LogLevel& loglevel)
{
    m_loglevel = loglevel;
}
void Logger::setConsoleHandlerLevel(const LogLevel& loglevel)
{
    m_consoleHandler.setLevel(loglevel);
}
void Logger::setFileHandlerLevel(const LogLevel& loglevel)
{
    m_fileHandler.setLevel(loglevel);
}


Logger& Logger::getLogger(const std::string& loggerName)
{
    if (Logger::s_loggers.contains(loggerName))
        return Logger::s_loggers[loggerName];

    Logger::s_loggers.insert(std::pair{ loggerName, loggerName });
    return Logger::s_loggers[loggerName];
}



LogHandler::LogHandler() { m_loglevel = LogLevel::LOG_INFO; }
LogHandler::LogHandler(const LogLevel& loglevel) { m_loglevel = loglevel; }
void LogHandler::setLevel(const LogLevel& loglevel) { m_loglevel = loglevel; }

void ConsoleHandler::log(const LogLevel& level, const char* message)
{
    if (level < m_loglevel) { return; }

    std::ostream& os = (level > LogLevel::LOG_DEBUG) ? std::cout : std::clog;
    const char* levelName = Logger::s_logLevelName.at(level);

    if (!isVirtual)
    {
        os << levelName << message << std::endl;
        return;
    }

    os << Styling::bold << levelName << Styling::reset;
    switch (level)
    {
    case LogLevel::LOG_INFO:
        os << Styling::fgCyan;
        break;
    case LogLevel::LOG_WARNING:
        os << Styling::bold << Styling::fgYellow;
        break;
    case LogLevel::LOG_ERROR:
        os << Styling::bold << Styling::fgRed;
        break;
    }

    os << message << Styling::reset << std::endl;
}

FileHandler::FileHandler(const std::filesystem::path& logdir, const LogLevel& loglevel)
{
    m_loglevel = loglevel;
    m_logdir = logdir;
}
Logging::FileHandler::~FileHandler()
{
    if (m_logfile.is_open())
        m_logfile.close();
}
void FileHandler::setLogDir(const std::filesystem::path& logdir) { m_logdir = logdir; }
void FileHandler::log(const LogLevel& level, const char* message)
{
    if (level < m_loglevel || m_fileError) { return; }

    if (!m_logdirChecked && !std::filesystem::exists(m_logdir) && !std::filesystem::is_directory(m_logdir))
    {
        if (!std::filesystem::create_directories(m_logdir)) {
            std::cerr << "###  Log Error: Could not create log directory \""
                << std::filesystem::absolute(m_logdir).string() << "\"  ###" << std::endl;
            return;
        }
    }
    m_logdirChecked = true;

    if (!m_logfileChecked)
    {
        std::filesystem::path filepath = m_logdir / "log.txt";
        m_logfile.open(filepath, std::ios::app);
        if (!m_logfile.is_open() || !m_logfile.good())
        {
            m_logfile.close();
            m_fileError = true;
            std::cerr << "###  Log Error: Could not create/open log file \""
                << std::filesystem::absolute(filepath).string() << "\"  ###" << std::endl;
        }
        m_logfileChecked = true;
    }

    m_logfile << Logger::s_logLevelName.at(level) << message << std::endl;
}
