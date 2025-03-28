#pragma once

#include <string>
#include <map>

namespace Logging
{
	enum class LogLevel
	{
		LOG_DEBUG,
		LOG_INFO,
		LOG_WARNING,
		LOG_ERROR,
	};

	extern LogLevel loglevel;

	static std::map<LogLevel, std::string> LogLevelName{
		{LogLevel::LOG_DEBUG, "DEBUG"},
		{LogLevel::LOG_INFO, "INFO"},
		{LogLevel::LOG_WARNING, "WARNING"},
		{LogLevel::LOG_ERROR, "ERROR"},
	};

	void log(const std::string& message, LogLevel level);
	void debug(const std::string& message);
	void info(const std::string& message);
	void warning(const std::string& message);
	void error(const std::string& message);
}
