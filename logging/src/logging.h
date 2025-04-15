#pragma once

#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <map>



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

#ifdef _DEBUG
	static inline const LogLevel DEFAULT_LOG_LEVEL = LogLevel::LOG_DEBUG;
#else
	static inline const LogLevel DEFAULT_LOG_LEVEL = LogLevel::LOG_INFO;
#endif

	class LogHandler
	{
	protected:
		LogLevel m_loglevel;
	public:
		LogHandler();
		LogHandler(const LogLevel&);
		void setLevel(const LogLevel&);
		virtual void log(const LogLevel&, const char*) = 0;
	};

	class ConsoleHandler : LogHandler
	{
	public:
		using LogHandler::LogHandler;
		using LogHandler::setLevel;
		void log(const LogLevel&, const char*) override;
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
		void log(const LogLevel&, const char*);
	};

	class Logger
	{
	public:
		static inline const std::map<LogLevel, const char*> s_logLevelName{
			{LogLevel::LOG_DEBUG,   "DEBUG:   "},
			{LogLevel::LOG_LOG,     ""         },
			{LogLevel::LOG_INFO,    "INFO:    "},
			{LogLevel::LOG_WARNING, "WARNING: "},
			{LogLevel::LOG_ERROR,   "ERROR:   "},
		};
		static Logger& getLogger(const std::string&);

		Logger(const std::string&);
		Logger() : Logger("logger") {}
		Logger(const Logger&) = delete;

		void debug(const char*);
		void log(const char*);
		void info(const char*);
		void warning(const char*);
		void warn(const char*);
		void error(const char*);

		void debug(const std::string&);
		void log(const std::string&);
		void info(const std::string&);
		void warning(const std::string&);
		void warn(const std::string&);
		void error(const std::string&);

		std::string getName() const;
		void setLevel(const LogLevel&);
		void setConsoleHandlerLevel(const LogLevel&);
		void setFileHandlerLevel(const LogLevel&);
	private:
		static inline std::map<std::string, Logger> s_loggers {};
		static inline ConsoleHandler s_defaultConsoleHandler { DEFAULT_LOG_LEVEL };
		static inline FileHandler s_defaultFileHandler;

		std::string m_name;
		LogLevel m_loglevel = DEFAULT_LOG_LEVEL;
		ConsoleHandler& m_consoleHandler = s_defaultConsoleHandler;
		FileHandler& m_fileHandler = s_defaultFileHandler;

		void _log(const LogLevel&, const char*);
	};


	namespace Styling
	{
		static inline const char* reset = "\033[0m";

		static inline const char* fgBlack =		"\033[30m";
		static inline const char* fgRed =		"\033[31m";
		static inline const char* fgGreen =		"\033[32m";
		static inline const char* fgYellow =	"\033[33m";
		static inline const char* fgBlue =		"\033[34m";
		static inline const char* fgMagenta =	"\033[35m";
		static inline const char* fgCyan =		"\033[36m";
		static inline const char* fgWhite =		"\033[37m";

		static inline const char* fgBrightBlack =	"\033[90m";
		static inline const char* fgBrightRed =		"\033[91m";
		static inline const char* fgBrightGreen =	"\033[92m";
		static inline const char* fgBrightYellow =	"\033[93m";
		static inline const char* fgBrightBlue =	"\033[94m";
		static inline const char* fgBrightMagenta =	"\033[95m";
		static inline const char* fgBrightCyan =	"\033[96m";
		static inline const char* fgBrightWhite =	"\033[97m";

		static inline const char* bold =			"\033[1m"; // works
		static inline const char* dim =				"\033[2m";
		static inline const char* italic =			"\033[3m";
		static inline const char* underline =		"\033[4m"; // works
		static inline const char* blinking =		"\033[5m";
		static inline const char* reverse =			"\033[6m";
		static inline const char* hidden =			"\033[8m";
		static inline const char* strikethrough =	"\033[8m";
	}

}
