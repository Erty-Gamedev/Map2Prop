#pragma once

#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <chrono>


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
	time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	struct tm timeinfo;
	localtime_s(&timeinfo, &t);

	std::stringstream buffer;
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

	extern inline const std::unordered_map<LogLevel, const char*> c_LOGLEVELNAME{
		{LogLevel::LOG_DEBUG,   "DEBUG:   "},
		{LogLevel::LOG_LOG,     ""         },
		{LogLevel::LOG_INFO,    "INFO:    "},
		{LogLevel::LOG_WARNING, "WARNING: "},
		{LogLevel::LOG_ERROR,   "ERROR:   "},
	};
	
	extern inline const std::unordered_map<LogLevel, const char*> c_LOGLEVELTITLE{
		{LogLevel::LOG_DEBUG,   "DEBUG"   },
		{LogLevel::LOG_LOG,     ""        },
		{LogLevel::LOG_INFO,    "INFO"    },
		{LogLevel::LOG_WARNING, "WARNING" },
		{LogLevel::LOG_ERROR,   "ERROR"   },
	};

#ifdef _DEBUG
	static inline const LogLevel DEFAULT_LOG_LEVEL = LogLevel::LOG_DEBUG;
#else
	static inline const LogLevel DEFAULT_LOG_LEVEL = LogLevel::LOG_INFO;
#endif

	extern bool g_isVirtual;

	class LogHandler
	{
	protected:
		LogLevel m_loglevel;
	public:
		LogHandler();
		LogHandler(const LogLevel&);
		void setLevel(const LogLevel&);
	};

	class ConsoleHandler : LogHandler
	{
	public:
		using LogHandler::LogHandler;
		using LogHandler::setLevel;

		template <typename T>
		void log(const LogLevel& level, T message)
		{
			if (level < m_loglevel) { return; }

			std::ostream& os = (level > LogLevel::LOG_DEBUG) ? std::cout : std::clog;
			const char* levelName = c_LOGLEVELNAME.at(level);

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

	class FileHandler : LogHandler
	{
	private:
		bool m_fileError = false;
		bool m_logdirChecked = false;
		bool m_logfileChecked = false;
		std::filesystem::path m_logdir;
		std::ofstream m_logfile;
	public:
		FileHandler(const std::filesystem::path&, const LogLevel&);
		FileHandler() : FileHandler("logs", LogLevel::LOG_WARNING) {}
		~FileHandler();
		using LogHandler::setLevel;
		void setLogDir(const std::filesystem::path&);

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

			m_logfile << formattedDatetime("[%FT%T]") << c_LOGLEVELTITLE.at(level) << "|"
				<< loggerName << "|" << message << std::endl;
		}
	};

	class Logger
	{
	public:
		static Logger& getLogger(const std::string&);

		Logger(const std::string&);
		Logger() : Logger("logger") {}
		Logger(const Logger&) = delete;

		template <typename T> void debug(T message) { _log(LogLevel::LOG_DEBUG, message); }
		template <typename T> void log(T message) { _log(LogLevel::LOG_LOG, message); }
		template <typename T> void info(T message) { _log(LogLevel::LOG_INFO, message); }
		template <typename T> void warning(T message) { _log(LogLevel::LOG_WARNING, message); }
		template <typename T> void warn(T message) { Logger::warning(message); }
		template <typename T> void error(T message) { _log(LogLevel::LOG_ERROR, message); }

		std::string getName() const;
		void setLevel(const LogLevel&);
		void setConsoleHandlerLevel(const LogLevel&);
		void setFileHandlerLevel(const LogLevel&);
	private:
		static inline std::unordered_map<std::string, Logger> s_loggers {};
		static inline ConsoleHandler s_defaultConsoleHandler { DEFAULT_LOG_LEVEL };
		static inline FileHandler s_defaultFileHandler;

		std::string m_name;
		LogLevel m_loglevel = DEFAULT_LOG_LEVEL;
		ConsoleHandler& m_consoleHandler = s_defaultConsoleHandler;
		FileHandler& m_fileHandler = s_defaultFileHandler;

		template <typename T>
		void _log(const LogLevel& level, T message)
		{
			if (level < m_loglevel) { return; }
			m_consoleHandler.log(level, message);
			m_fileHandler.log(level, message, m_name);
		}
	};

}
