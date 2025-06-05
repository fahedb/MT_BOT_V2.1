#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <codecvt>
#include <locale>
#include <cctype>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>

enum TimeUnits
{
	Sec = 1,
	Min = 60,
	Hour = 3600,
	Day = 86400
};

class ProcessData
{
public:
	static void OpenA_Folder(std::string folderName);
	static std::string GetFormattedCurrentDate(char seperator = '-' /*Defualt value is - */);
	static std::string GetFormattedCurrTime(char seperator = ':' /*defualt value is : */);

	static std::string TrimSpaces(const std::string& input);
	static void ConvertSQLCHARToString(const SQLCHAR* sqlCharData,std::string& outputString);
	static std::string SQLWCHARToString(SQLWCHAR* sqlwcharMessage);
	static int roundToDecimalPlaces(double value, int decimalPlaces);
	static double convertDecimalToDouble(int value, int decimalPlaces);

	static std::vector<std::string> splitString(const std::string& str, std::string splitBy = ",");
};

