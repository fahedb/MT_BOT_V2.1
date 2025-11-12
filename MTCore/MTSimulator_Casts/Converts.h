#pragma once
#include <string>

// include for cout 
#include <iostream>

class Converts
{
public:
	static int ToInt(const std::string& str)
	{
		try {
			return std::stoi(str);
		}
		catch (const std::exception& e) {
			std::cerr << "Invalid int conversion: " << str << " | " << e.what() << std::endl;
			return 0;
		}
	}

	static long ToLong(const std::string& str)
	{
		try {
			return std::stol(str);
		}
		catch (const std::exception& e) {
			std::cerr << "Invalid long conversion: " << str << " | " << e.what() << std::endl;
			return 0;
		}
	}
	static long long ToLongLong(const std::string& str)
	{
		try {
			return std::stoll(str);
		}
		catch (const std::exception& e) {
			std::cerr << "Invalid long long conversion: " << str << " | " << e.what() << std::endl;
			return 0;
		}
	}
	static double ToDouble(const std::string& str)
	{
		try {
			return std::stod(str);
		}
		catch (const std::exception& e) {
			std::cerr << "Invalid double conversion: " << str << " | " << e.what() << std::endl;
			return 0.0;
		}
	}
};

