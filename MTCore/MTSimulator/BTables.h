
#include "ProcessData.h"
#include "Configrations.h"
#include "Logger.h"
#include "nlohmann/json.hpp"
#include <queue>
#include <mutex>

#pragma once

#ifndef BTables_HEADER
#define BTables_HEADER
//#include "ProcessData.h"
#define JSON_SLTRING_LEN 570
class JsonSelectionNode
{
	nlohmann::json_abi_v3_11_3::json m_selection;
public:
	bool m_valid = false;
	JsonSelectionNode() { m_valid = false; }
	JsonSelectionNode(nlohmann::json_abi_v3_11_3::json j) 
	{
		m_selection = j;
		m_valid = true;
	}
	nlohmann::json_abi_v3_11_3::json* Get_Json() { return &m_selection; }
};

class SQLSelectionNode
{

	std::vector<std::vector<SQLCHAR>> m_row;

public:
	SQLSelectionNode(const std::vector<std::vector<SQLCHAR>>& row): m_row(row) {}
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


class BTables
{
private:
	SQLHDBC m_DBConnectionHandler = 0;
	SQLHENV m_DBenvHandler = 0;
	std::vector<SQLSelectionNode*> m_allSelectedRows;
	std::vector<FileSelectionNode*> m_allSelectedFileRows;
	std::mutex queueMutex;
	std::queue<JsonSelectionNode*> m_allSelectedJsonRows;
	std::vector<std::vector<JsonSelectionNode*>> m_allSelectedJsonRows_MultiThreaded;
	std::vector<int> m_jsonsThreadIdxsPlaces;
	bool m_multiThreadedRead;
	std::string m_fileName;
public: 
	BTables(SQLHENV& henv, SQLHDBC& hdbc)
	{
		m_DBConnectionHandler = hdbc;
		m_DBenvHandler = henv;
		m_fileName = "";
		m_allSelectedRows.clear();
		m_allSelectedFileRows.clear();
		m_multiThreadedRead = false;
	}
	BTables(std::string filename)
	{
		m_fileName = filename;
		m_allSelectedRows.clear();
		m_allSelectedFileRows.clear();
		m_multiThreadedRead = false;
	}
	BTables()
	{
		m_allSelectedRows.clear();
		m_allSelectedFileRows.clear();
		m_multiThreadedRead = false;
	}

protected:
	long GetFileLinesCount(const std::string& filePath);
	static std::vector<JsonSelectionNode*> FileChunkRead(int startLineIdx, int endLineIdx, int threadIdx,const std::string fileName);
	void Set_DBFileName(std::string fileName) { m_fileName = fileName; }
	void Set_AsMultiThread_Read() { m_multiThreadedRead = true; }
	virtual std::string GetDB_PhysicalFileName() { return Configrations::getInstance()->Get_ConfigValue(DBFILE_DEFUALT_DB_FILE); }
	bool SelectFromFile(int cols);
	bool SelectFromJsonFile();
	bool SelectFromJsonFile_MultiThreaded();
	std::vector<SQLSelectionNode*>* GetSelectedRows() { return &m_allSelectedRows; }
	int Get_SelectedCount() {
		if (m_allSelectedJsonRows.size())
			return m_allSelectedJsonRows.size();
		if (m_allSelectedRows.size())
			return m_allSelectedRows.size();
		if (m_allSelectedFileRows.size())
			return m_allSelectedFileRows.size();
		return 0;
	}
	int Get_SelectedCount_MultiThreaded(int threadIdx)
	{
		if (m_allSelectedJsonRows_MultiThreaded.size() >= threadIdx)
		{
			if (m_allSelectedJsonRows_MultiThreaded.at(threadIdx).size())
				return m_allSelectedJsonRows_MultiThreaded.at(threadIdx).size();
		}
		return 0;
	}
	std::vector<FileSelectionNode*>* GetSelectedRowsFromFile() { return &m_allSelectedFileRows; }
	JsonSelectionNode* GetNextJsonSelection() 
	{
		if (!m_allSelectedJsonRows.size())
			return nullptr;
		JsonSelectionNode* j = m_allSelectedJsonRows.front();
		m_allSelectedJsonRows.pop();
		return j;
	}
	JsonSelectionNode* GetNextJsonSelection_MultiThreaded(int threadIdx)
	{
		if (threadIdx >= m_allSelectedJsonRows_MultiThreaded.size())
			return nullptr;
		
		size_t currIdx = m_jsonsThreadIdxsPlaces[threadIdx];
		auto& jsonRows = m_allSelectedJsonRows_MultiThreaded[threadIdx];

		if (currIdx >= jsonRows.size())
			return nullptr;

		JsonSelectionNode* j = jsonRows[currIdx];
		m_jsonsThreadIdxsPlaces[threadIdx] = currIdx + 1;
		return j;

	}
	bool InsertIntoDB(std::string insertSQL);
	bool SelectFromDB(std::string selectSQL,int cols,int colLen = 24);
	bool FreeStyleSelectFromDB(std::string selectSQL, int cols) { return SelectFromDB(selectSQL, cols); }
	bool UpdateDB(std::string insertSQL);
	bool DBCommand(std::string commandSQL) { return UpdateDB(commandSQL); }

	void Insert_NewBody_to_queue(JsonSelectionNode* node) { m_allSelectedJsonRows.push(node); }

	void RemoveSelection()
	{
		for (int i = 0; i < m_allSelectedRows.size(); i ++ )
		{
			delete m_allSelectedRows.at(i);
		}
		m_allSelectedRows.clear();

		for (int i = 0; i < m_allSelectedFileRows.size(); i++)
		{
			delete m_allSelectedFileRows.at(i);
		}
		m_allSelectedFileRows.clear();

		for (int i = 0; i < m_allSelectedJsonRows.size(); i++)
		{
			JsonSelectionNode* j = m_allSelectedJsonRows.front();
			delete j;
			j = nullptr;
			m_allSelectedJsonRows.pop();
		}

		for (int i = 0; i < m_allSelectedJsonRows_MultiThreaded.size(); i++)
		{
			for (int t = 0; t < m_allSelectedJsonRows_MultiThreaded.at(i).size(); t++)
			{
				JsonSelectionNode* j = m_allSelectedJsonRows_MultiThreaded.at(i).at(t);
				delete j;
			}
			m_allSelectedJsonRows_MultiThreaded.at(i).clear();
		}
		m_allSelectedJsonRows_MultiThreaded.clear();
	}
};

#endif