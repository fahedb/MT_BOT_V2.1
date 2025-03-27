#include "ProcessData.h"

void ProcessData::OpenA_Folder(std::string folderName)
{
	// Convert std::string to std::wstring because Windows API functions often use wide characters
	std::wstring wideFolderName(folderName.begin(), folderName.end());

	// Create the directory
	BOOL result = CreateDirectoryW(wideFolderName.c_str(), nullptr);

	if (result == 0) {
		// If creation fails, use GetLastError to get more info about the error
		DWORD error = GetLastError();
		if (error == ERROR_ALREADY_EXISTS) {
			std::cerr << "The folder already exists." << std::endl;
		}
		else {
			std::cerr << "Failed to create folder. Error code: " << error << std::endl;
		}
	}
	else {
		std::cout << "Folder created successfully!" << std::endl;
	}
}

std::string ProcessData::GetFormattedCurrentDate(char seperator) {
	// Get current time
	std::time_t t = std::time(nullptr);
	std::tm tm;

	// Thread-safe local time conversion
	errno_t err = localtime_s(&tm, &t);
	if (err != 0) {
		throw std::runtime_error("Failed to get local time");
	}

	// Format date as YYYY-MM-DD
	std::ostringstream oss;
	std::string format = "%Y-%m-%d";
	std::replace(format.begin(), format.end(), '-', seperator);
	oss << std::put_time(&tm, format.c_str());
	return oss.str();
}

std::string ProcessData::GetFormattedCurrTime(char seperator)
{
	// Get current time
	std::time_t t = std::time(nullptr);
	std::tm tm; // Thread-safe structure for time
	errno_t err = localtime_s(&tm, &t);
	if (err != 0) {
		throw std::runtime_error("Failed to get local time");
	}

	// Format time as HH:MM:SS
	std::ostringstream oss;
	std::string format = "%H:%M";
	std::replace(format.begin(), format.end(), ':', seperator);
	oss << std::put_time(&tm, format.c_str());
	return oss.str();
}

std::string ProcessData::TrimSpaces(const std::string& input) {
	// Find the first non-space character
	auto start = std::find_if_not(input.begin(), input.end(), [](unsigned char ch) {
		return std::isspace(ch);
		});

	// Find the last non-space character
	auto end = std::find_if_not(input.rbegin(), input.rend(), [](unsigned char ch) {
		return std::isspace(ch);
		}).base();

		// If there are no non-space characters, return an empty string
		if (start >= end) {
			return "";
		}

		// Construct the trimmed string
		return std::string(start, end);
}

void ProcessData::ConvertSQLCHARToString(const SQLCHAR* sqlCharData, std::string& outputString) {
	// Convert SQLCHAR* to std::string
	outputString = reinterpret_cast<const char*>(sqlCharData);
}

std::string ProcessData::SQLWCHARToString(SQLWCHAR* sqlwcharMessage) {
	// Convert wide characters to a std::wstring
	std::wstring wstr(sqlwcharMessage);

	// Convert std::wstring to std::string (UTF-8 encoded)
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);
}

int ProcessData::roundToDecimalPlaces(double value, int decimalPlaces) {
	double factor = std::pow(10.0, decimalPlaces);
	int retVal = static_cast<double>(value) * static_cast<double>(factor);
	retVal = static_cast<int>(std::round(retVal));
	return retVal;
}

double ProcessData::convertDecimalToDouble(int value, int decimalPlaces)
{
	double factor = std::pow(10.0, decimalPlaces);
	double retVal = value / factor;
	return retVal;
}

std::vector<std::string> ProcessData::splitString(const std::string& str, std::string splitBy)
{
	std::string currStr = "";
	std::vector<std::string> record;

	for (int i = 0; i < str.length(); i++)
	{
		if (str[i] == splitBy[0])
		{
			record.push_back(currStr);
			currStr = "";
			i++;
		}
		currStr += str[i];

	}
	record.push_back(currStr);
	return record;
}
