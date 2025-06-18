#include "SelectionNodes.h"

#include "ProcessData.h"
#include "Configrations.h"
#include "Logger.h"
#include <queue>
#include <mutex>

#pragma once

#ifndef BTables_HEADER
#define BTables_HEADER
//#include "ProcessData.h"
#define JSON_SLTRING_LEN 3400


class BTables
{
private:
	SQLHDBC m_DBConnectionHandler = 0;
	SQLHENV m_DBenvHandler = 0;
	std::vector<SQLSelectionNode*> m_allSelectedRows;
	std::vector<FileSelectionNode*> m_allSelectedFileRows;
	std::queue<JsonSelectionNode*> m_allSelectedJsonRows;
	std::vector<std::vector<JsonSelectionNode*>> m_allSelectedJsonRows_MultiThreaded;
	std::vector<int> m_jsonsThreadIdxsPlaces;
	bool m_multiThreadedRead;

	std::mutex queueMutex;
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
	BTables(const std::string& filename)
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

protected: // Functions to control text file

	long GetFileLinesCount(const std::string& filePath);
	bool SelectFromFile(int cols);
	std::vector<SQLSelectionNode*>* GetSelectedRows() { return &m_allSelectedRows; }
	std::vector<FileSelectionNode*>* GetSelectedRowsFromFile() { return &m_allSelectedFileRows; }
	virtual std::string GetDB_PhysicalFileName() { return Configrations::getInstance()->Get_ConfigValue(DBFILE_DEFUALT_DB_FILE); }
	virtual std::string GetDB_PhysicalFileName_Json() { return Configrations::getInstance()->Get_ConfigValue(DBFILE_DEFUALT_DB_FILE_JSON); }
	void Set_DBFileName(std::string fileName) { m_fileName = fileName; }

protected: // Function to Control json file
	void Set_AsMultiThread_Read() { m_multiThreadedRead = true; }
	static std::vector<JsonSelectionNode*> FileChunkRead(int startLineIdx, int endLineIdx, int threadIdx,const std::string& fileName);
	int Get_SelectedCount_MultiThreaded(int threadIdx) { return m_allSelectedJsonRows_MultiThreaded.size() > static_cast<size_t>(threadIdx) ? static_cast<int>(m_allSelectedJsonRows_MultiThreaded.at(threadIdx).size()) : 0; }
	bool SelectFromJsonFile();
	bool SelectFromJsonFile_MultiThreaded(); // Select from a json file, with multithreading, need to call removeselection after done using 
	JsonSelectionNode* GetNextJsonSelection(); 
	JsonSelectionNode* GetNextJsonSelection_MultiThreaded(int threadIdx);


protected: // Function to control SQL DataBase

	bool InsertIntoDB(const std::string& insertSQL);
	bool SelectFromDB(const std::string& selectSQL,int cols,int colLen = 24);
	bool UpdateDB(const std::string& insertSQL);

	void Insert_NewBody_to_queue(JsonSelectionNode* node) { m_allSelectedJsonRows.push(node); }
	void RemoveSelection();
private:
	void RemoveSelectionPerThread(int threadIdx);
};

#endif