#include <string>
#include "LOGGER.H"
#pragma once
class Converts
{
public:
	static int ToInt(const std::string& str)
	{
		try {
			return std::stoi(str);
		}
		catch (const std::exception& e) {
			Logger::getInstance().WriteLog("Invalid int conversion: " + str + " | " + e.what(), false, true, LogType::ERR);
			return 0;
		}
	}

	static long ToLong(const std::string& str)
	{
		try {
			return std::stol(str);
		}
		catch (const std::exception& e) {
			Logger::getInstance().WriteLog("Invalid int conversion: " + str + " | " + e.what(), false, true, LogType::ERR);
			return 0;
		}
	}
	static long long ToLongLong(const std::string& str)
	{
		try {
			return std::stoll(str);
		}
		catch (const std::exception& e) {
			Logger::getInstance().WriteLog("Invalid int conversion: " + str + " | " + e.what(), false, true, LogType::ERR);
			return 0;
		}
	}
	static double ToDouble(const std::string& str)
	{
		try {
			return std::stod(str);
		}
		catch (const std::exception& e) {
			Logger::getInstance().WriteLog("Invalid double conversion: " + str + " | " + e.what(), false, true, LogType::ERR);
			return 0.0;
		}
	}
};

