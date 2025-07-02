#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>

#pragma once
#include "nlohmann/json.hpp"

class JsonSelectionNode
{
	nlohmann::json m_selection;
public:
	bool m_valid = false;
	JsonSelectionNode() { m_valid = false; }
	JsonSelectionNode(nlohmann::json j)
	{
		m_selection = j;
		m_valid = true;
	}
	nlohmann::json* Get_Json() { return &m_selection; }
};

class SQLSelectionNode
{

	std::vector<std::vector<SQLCHAR>> m_row;

public:
	SQLSelectionNode(const std::vector<std::vector<SQLCHAR>>& row) : m_row(row) {}
	~SQLSelectionNode() {
		for (int i = 0; i < m_row.size(); i++)
		{
			auto& aWord = m_row.at(i);
			aWord.clear();
		}
		m_row.clear();
	}
	const std::vector<std::vector<SQLCHAR>>* GetRow()	const { return &m_row; }
	std::string GetColumn(int i) {
		//static void ConvertSQLCHARToString(const SQLCHAR * sqlCharData, std::string & outputString) {

		std::string outputString;
		outputString = reinterpret_cast<const char*>(m_row.at(i).data());
		return outputString;
	}
};


class FileSelectionNode
{

	std::vector<std::string> m_row;

public:
	FileSelectionNode(const std::vector<std::string>& row) : m_row(row) {}
	const std::vector<std::string>* GetColumn()	const { return &m_row; }
	const std::string GetColumn(int idx) const { return m_row.at(idx); }
	void Clean() { m_row.clear(); }
};
