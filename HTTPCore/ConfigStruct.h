#pragma once
#include <string>


struct ConfigStruct
{
	std::string label;
	std::string value;
	std::string explain;
	std::string policyCategory;
	int m_permission = 0;
	int m_catagory = 0;
	bool need_To_Update = false;
};
