#include "SelectionNodes.h"
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

	int m_totalFileLines = 0;

	std::mutex queueMutex;
	std::string m_fileName;

	HANDLE m_hFile = nullptr;
	HANDLE m_hMapping = nullptr;
	char* m_startCharPtr = nullptr;
	char* m_endCharPtr = nullptr;
	std::vector<std::pair<char*, char*>> m_currFileIdxPlaces;
	std::vector<int> m_currLineIdx;
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

	long GetFileLinesCount();
	std::vector<SQLSelectionNode*>* GetSelectedRows() { return &m_allSelectedRows; }
	std::vector<FileSelectionNode*>* GetSelectedRowsFromFile() { return &m_allSelectedFileRows; }
	virtual std::string GetDB_PhysicalFileName() = 0;
	virtual std::string GetDB_PhysicalFileName_Json() = 0;
	void Set_DBFileName(std::string fileName) { m_fileName = fileName; }

protected: // Function to Control json file
	void Set_AsMultiThread_Read() { m_multiThreadedRead = true; }
	void UnloadFileMapping()
	{
		UnmapViewOfFile(m_startCharPtr);
		CloseHandle(m_hFile);
		CloseHandle(m_hMapping);

		m_startCharPtr = nullptr;
		m_hFile = nullptr;
		m_hMapping = nullptr;
	}
	bool InitFileMap();
	JsonSelectionNode* GetNextJsonSelection();
	void InitLineIdx(int startLineIdx, int endLineIdx, int threadIdx)
	{
		// Look for the relevant line
		int lineCount = startLineIdx;

		char* currPtr = threadIdx > 0 ? m_currFileIdxPlaces.at(threadIdx - 1).second : m_startCharPtr;
		char* endPtr = m_endCharPtr;
		m_currFileIdxPlaces.at(threadIdx).first = currPtr; // Init the start ptr

		if (*currPtr == '\n') currPtr++;

		while (currPtr < endPtr && lineCount < endLineIdx) {
			if (*currPtr == '\n') lineCount++;
			currPtr++;
		}
		m_currFileIdxPlaces.at(threadIdx).second = currPtr; // Init the start ptr

	}

	JsonSelectionNode* GetNextSelection_newLogic(int threadIdx)
	{
		char* startPtr = m_currFileIdxPlaces.at(threadIdx).first;
		char* endPtr = m_currFileIdxPlaces.size() - 1 > threadIdx ? m_currFileIdxPlaces.at(threadIdx).second : m_endCharPtr;
		//JsonSelectionNode* j = nullptr;
		std::string currentLine;
		currentLine.reserve(JSON_SLTRING_LEN);
		if (*startPtr == '\n')
			startPtr++;
		while (startPtr < endPtr)
		{
			if (*startPtr == '\n') {
				startPtr++; // To skip the new line in the next run
				m_currFileIdxPlaces.at(threadIdx).first = startPtr;
				return new JsonSelectionNode(nlohmann::json::parse(currentLine));

			}
			else {
				currentLine += *startPtr;
			}
			startPtr++;
		}
		return nullptr;
	}

protected: // Function to control SQL DataBase

	bool InsertIntoDB(const std::string& insertSQL);
	bool SelectFromDB(const std::string& selectSQL,int cols,int colLen = 24);
	bool UpdateDB(const std::string& insertSQL);

	void Insert_NewBody_to_queue(JsonSelectionNode* node) { m_allSelectedJsonRows.push(node); }
	void RemoveSelection();
	int GetBeginGap();
private:
	// Remove json allocations per thread, this function will work in chunks, after finishing the chunk it will enqueue new task
};

#endif