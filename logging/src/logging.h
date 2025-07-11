#pragma once

#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <chrono>

#ifdef _WIN32
#include <wchar.h>
#include <windows.h>
#endif


/**
 * Check if we can enable virtual terminal (needed for ANSI escape sequences)
 * From: https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#example-of-enabling-virtual-terminal-processing
 */
static inline bool enableVirtualTerminal()
{
#ifdef _WIN32
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
#endif
	return true;
}


namespace Styling
{
	static inline const char* reset = "\033[0m";

	static inline const char* fgBlack = "\033[30m";
	static inline const char* fgRed = "\033[31m";
	static inline const char* fgGreen = "\033[32m";
	static inline const char* fgYellow = "\033[33m";
	static inline const char* fgBlue = "\033[34m";
	static inline const char* fgMagenta = "\033[35m";
	static inline const char* fgCyan = "\033[36m";
	static inline const char* fgWhite = "\033[37m";

	static inline const char* fgBrightBlack = "\033[90m";
	static inline const char* fgBrightRed = "\033[91m";
	static inline const char* fgBrightGreen = "\033[92m";
	static inline const char* fgBrightYellow = "\033[93m";
	static inline const char* fgBrightBlue = "\033[94m";
	static inline const char* fgBrightMagenta = "\033[95m";
	static inline const char* fgBrightCyan = "\033[96m";
	static inline const char* fgBrightWhite = "\033[97m";

	static inline const char* bold = "\033[1m"; // works
	static inline const char* dim = "\033[2m";
	static inline const char* italic = "\033[3m";
	static inline const char* underline = "\033[4m"; // works
	static inline const char* blinking = "\033[5m";
	static inline const char* reverse = "\033[6m";
	static inline const char* hidden = "\033[8m";
	static inline const char* strikethrough = "\033[8m";
}


static inline std::string formattedDatetime(const char* fmt)
{
	std::stringstream buffer;
	time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	struct tm timeinfo;
#ifdef _WIN32
	localtime_s(&timeinfo, &t);
#else
	localtime_r(&t, &timeinfo);
#endif
	buffer << std::put_time(&timeinfo, fmt);
	return buffer.str();
}


namespace Logging
{
	enum class LogLevel
	{
		LOG_DEBUG,
		LOG_INFO,
		LOG_LOG,
		LOG_WARNING,
		LOG_ERROR,
	};

	inline const char* getLogLevelName(LogLevel level, bool titleOnly = false)
	{
		if (titleOnly)
		{
			switch (level)
			{
			case LogLevel::LOG_DEBUG:   return "DEBUG";
			case LogLevel::LOG_LOG:     return "";
			case LogLevel::LOG_INFO:    return "INFO";
			case LogLevel::LOG_WARNING: return "WARNING";
			case LogLevel::LOG_ERROR:   return "ERROR";
			}
		}
		else
		{
			switch (level)
			{
			case LogLevel::LOG_DEBUG:   return "DEBUG:   ";
			case LogLevel::LOG_LOG:     return "";
			case LogLevel::LOG_INFO:    return "INFO:    ";
			case LogLevel::LOG_WARNING: return "WARNING: ";
			case LogLevel::LOG_ERROR:   return "ERROR:   ";
			}
		}
		return "";
	}

#ifdef _DEBUG
	static inline constexpr LogLevel DEFAULT_LOG_LEVEL = LogLevel::LOG_DEBUG;
#else
	static inline constexpr LogLevel DEFAULT_LOG_LEVEL = LogLevel::LOG_INFO;
#endif

	static bool g_isVirtual = enableVirtualTerminal();

	class LogHandler
	{
	protected:
		LogLevel m_loglevel;
	public:
		LogHandler() : m_loglevel(LogLevel::LOG_INFO) {}
		LogHandler(const LogLevel& loglevel) : m_loglevel(loglevel) {}
		void setLevel(const LogLevel& loglevel) { m_loglevel = loglevel; }
	};

	class ConsoleHandler : public LogHandler
	{
	public:
		using LogHandler::LogHandler;
		using LogHandler::setLevel;

		template <typename T>
		void log(const LogLevel& level, T message)
		{
			if (level < m_loglevel) { return; }

			std::ostream& os = (level > LogLevel::LOG_INFO) ? std::cerr : std::cout;
			const char* levelName = getLogLevelName(level);

			if (!g_isVirtual)
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
	};

	class FileHandler : public LogHandler
	{
	private:
		bool m_fileError = false;
		bool m_logdirChecked = false;
		bool m_logfileChecked = false;
		std::filesystem::path m_logdir;
		std::ofstream m_logfile;
	public:
		FileHandler(const std::filesystem::path& logdir, const LogLevel& loglevel) : m_logdir(logdir) { m_loglevel = loglevel; }
		FileHandler() : FileHandler("logs", LogLevel::LOG_WARNING) {}
		~FileHandler() { if (m_logfile.is_open()) m_logfile.close(); }
		using LogHandler::setLevel;
		void setLogDir(const std::filesystem::path& logdir) { m_logdir = logdir; }

		template <typename T>
		void log(const LogLevel& level, T message, std::string loggerName)
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
				std::filesystem::path filepath = m_logdir / formattedDatetime("log_%F.txt");
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

			m_logfile << formattedDatetime("[%FT%T]") << getLogLevelName(level, true) << "|"
				<< loggerName << "|" << message << std::endl;
		}
	};

	class Logger
	{
	public:
		Logger(const std::string& name) : m_name(name), m_loglevel(DEFAULT_LOG_LEVEL) {}
		Logger() : Logger("logger") {}
		Logger(const Logger&) = delete;

		template <typename T> void debug(T message) { _log(LogLevel::LOG_DEBUG, message); }
		template <typename T> void log(T message) { _log(LogLevel::LOG_LOG, message); }
		template <typename T> void info(T message) { _log(LogLevel::LOG_INFO, message); }
		template <typename T> void warning(T message) { _log(LogLevel::LOG_WARNING, message); }
		template <typename T> void warn(T message) { Logger::warning(message); }
		template <typename T> void error(T message) { _log(LogLevel::LOG_ERROR, message); }

		std::string getName() const { return m_name; }
		void setLevel(const LogLevel& loglevel) { m_loglevel = loglevel; }
		void setConsoleHandlerLevel(const LogLevel& loglevel) { m_consoleHandler.setLevel(loglevel); }
		void setFileHandlerLevel(const LogLevel& loglevel) { m_fileHandler.setLevel(loglevel); }
		void setFileHandlerLogDir(const std::filesystem::path& logDir) { m_fileHandler.setLogDir(logDir); }

		static Logger& getLogger(const std::string& loggerName)
		{
			if (Logger::s_loggers.contains(loggerName)) return *Logger::s_loggers[loggerName];
			Logger::s_loggers.insert(std::pair{ loggerName, std::make_unique<Logger>(loggerName) });
			return *Logger::s_loggers[loggerName];
		}
		static void setGlobalConsoleLevelDebug()
		{
			s_defaultConsoleHandler.setLevel(LogLevel::LOG_DEBUG);
			for (std::pair<const std::string, std::unique_ptr<Logger>>& kv : s_loggers)
			{
				kv.second->setLevel(LogLevel::LOG_DEBUG);
				kv.second->setConsoleHandlerLevel(LogLevel::LOG_DEBUG);
			}
		}
	private:
		static inline std::unordered_map<std::string, std::unique_ptr<Logger>> s_loggers{};
		static inline ConsoleHandler s_defaultConsoleHandler{ DEFAULT_LOG_LEVEL };
		static inline FileHandler s_defaultFileHandler;

		const std::string m_name;
		LogLevel m_loglevel = DEFAULT_LOG_LEVEL;
		ConsoleHandler& m_consoleHandler = s_defaultConsoleHandler;
		FileHandler& m_fileHandler = s_defaultFileHandler;

		template <typename T> void _log(const LogLevel& level, T message)
		{
			if (level < m_loglevel) { return; }
			m_consoleHandler.log(level, message);
			m_fileHandler.log(level, message, m_name);
		}
	};
}
