#include "BTables.h"
#include <iomanip>
#include <map>
#include <future>      // std::async, std::future
#include "ThreadPool.h"
#include "ProcessData.h"
#include "ProccessFlags.h"
#include "Converts.h"
#include "Logger.h"



bool BTables::InsertIntoDB(const std::string& insertSQL)
{
	if (!m_DBConnectionHandler || !m_DBenvHandler)
		return false;

	SQLHSTMT hstmt;
	SQLAllocHandle(SQL_HANDLE_STMT, m_DBConnectionHandler, &hstmt);
	
	// Define the INSERT SQL command
	//std::string insertSQL = "INSERT INTO EURUSD_TimeLinePrices (PriceDateTime, BuyPrice,SellPrice,ActionSubmitted) VALUES ('1', 1,1,1)";

	// Convert SQL command to wide string
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::wstring wide_str_insertSQL = converter.from_bytes(insertSQL);
	SQLWCHAR* sqlCommand = const_cast<SQLWCHAR*>(wide_str_insertSQL.c_str());

	// Execute the SQL command
	int retcode = SQLExecDirect(hstmt, sqlCommand, SQL_NTS);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	if (SQL_SUCCEEDED(retcode)) {
		//std::cout << "Insert successful.\n";
		return true;
	}
	else {
		//std::cout << "Insert failed.\n";
		return false;
		std::cout << "insert failed" << std::endl;
	}
}

bool BTables::SelectFromDB(const std::string& selectSQL,int cols,int colLen)
{
	if (!m_DBConnectionHandler || !m_DBenvHandler)
		return false;

	SQLHSTMT hstmt;

	// Allocate statement handle
	SQLRETURN res = SQLAllocHandle(SQL_HANDLE_STMT, m_DBConnectionHandler, &hstmt);
	if (!hstmt)
		return false;

	// Convert SQL command to wide string
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::wstring wide_str_QuerySQL = converter.from_bytes(selectSQL);
	SQLWCHAR* sqlCommand = const_cast<SQLWCHAR*>(wide_str_QuerySQL.c_str());

	// Execute the SQL command
	int retcode = SQLExecDirect(hstmt, sqlCommand, SQL_NTS);

	if (SQL_SUCCEEDED(retcode)) {
		// Fetch and display results

		SQLLEN indicator;
		int count = 1;
		while (SQLFetch(hstmt) == SQL_SUCCESS) {
			std::vector<std::vector<SQLCHAR>> currRow(cols, std::vector<SQLCHAR>(colLen));
			for (int i = 0; i < cols; i++)
			{
				SQLGetData(hstmt, i + 1, SQL_C_CHAR, currRow.at(i).data(), colLen, &indicator);
			}
			m_allSelectedRows.push_back(new SQLSelectionNode(currRow));
			
		}

	}
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	return true;
}

bool BTables::UpdateDB(const std::string& insertSQL) {
	if (!m_DBConnectionHandler || !m_DBenvHandler)
		return false;

	SQLHSTMT hstmt; 
	SQLAllocHandle(SQL_HANDLE_STMT, m_DBConnectionHandler, &hstmt);

	// Convert SQL command to wide string
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::wstring wide_str_insertSQL = converter.from_bytes(insertSQL);
	SQLWCHAR* sqlCommand = const_cast<SQLWCHAR*>(wide_str_insertSQL.c_str());

	// Execute the SQL command
	int retcode = SQLExecDirect(hstmt, sqlCommand, SQL_NTS);
	if (SQL_SUCCEEDED(retcode)) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

		return true;
	}
	else {
		std::cout << "update failed" << std::endl;
		// Retrieve error details
		SQLWCHAR sqlState[SQL_SQLSTATE_SIZE + 1];
		SQLWCHAR message[SQL_MAX_MESSAGE_LENGTH];
		SQLINTEGER nativeError;
		SQLSMALLINT messageLength;
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

		return false;

	}
}

void BTables::RemoveSelection()
{
	for (int i = 0; i < m_allSelectedRows.size(); i++)
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
	UnloadFileMapping();
}

int BTables::GetBeginGap()
{
	int beginLineGap = 0;
	std::string skipValue = ProccessFlags::GetSwitch(LIMIT_FILE_LINES_LOAD);
	if (skipValue.length() || LIMIT_LAST_X_LINES)
	{
		int linesCount = GetFileLinesCount();
		if (!skipValue.length())
			skipValue = std::to_string(LIMIT_LAST_X_LINES);
		int dataLimit = Converts::ToInt(skipValue);

		if (dataLimit > 0 && dataLimit < linesCount)
		{
			// Calc what is the gap needed to skip till the last LIMIT param lines
			beginLineGap = linesCount - dataLimit;
		}
	}
	return beginLineGap;
}



bool BTables::InitFileMap()
{
	// Open the file
	HANDLE hFile = CreateFileA(m_fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		Logger::getInstance().WriteLog("Error opening file: " + m_fileName, false, true, LogType::ERR);
		CloseHandle(hFile);
		return false;
	}

	// Create file Mapping
	HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!hMapping) {
		CloseHandle(hFile);
		Logger::getInstance().WriteLog("Error opening file: " + m_fileName, false, true, LogType::ERR);
		return false;
	}

	// Map to memory
	char* fileData = static_cast<char*>(MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0));
	if (!fileData) {
		CloseHandle(hMapping);
		CloseHandle(hFile);
		std::cerr << "Error mapping file" << std::endl;
		return false;
	}

	// Size of file
	LARGE_INTEGER fileSize;

	if (!GetFileSizeEx(hFile, &fileSize)) {
		UnmapViewOfFile(fileData);
		CloseHandle(hMapping);
		CloseHandle(hFile);
		std::cerr << "Error getting file size" << std::endl;
		return false;
	}

	m_hFile = hFile;
	m_hMapping = hMapping;

	m_startCharPtr = fileData;
	m_endCharPtr = fileData + fileSize.QuadPart;

	int beginGap = GetBeginGap();
	
	if (beginGap > 0)
	{
		int totalLineCount = GetFileLinesCount(); // Ensure we have the total lines count
		if(beginGap >= totalLineCount)
			beginGap = 0; // Reset gap if it exceeds total lines
		// Move the fileData pointer to the begin gap
		char* currPtr = fileData;
		char* endPtr = fileData + fileSize.QuadPart;
		int lineCount = 0;
		while (currPtr < endPtr && lineCount < beginGap) {
			if (*currPtr == '\n') lineCount++;
			currPtr++;
		}
		fileData = currPtr;
	}
	m_totalFileLines = 0; // Reset total lines count to force recount

	m_startCharPtr = fileData;
	int linesCount = GetFileLinesCount();
	int threadsNum = MTThreadPool::getInstance()->Get_ThreadsNum();

	int chunkSize = static_cast<long>(linesCount) / threadsNum;
	for (int i = 0; i < threadsNum; i++)
	{
		m_currFileIdxPlaces.push_back(std::make_pair(fileData, fileData));
		m_lineCounterPerThread.push_back(i * chunkSize);
	}
}

long BTables::GetFileLinesCount()
{
	if(m_totalFileLines > 0)
		return m_totalFileLines;
	// Count lines by iterating through file data
	long lineCount = 0;
	char* startPtr = m_startCharPtr;
	char* endPtr = m_endCharPtr;
	if (startPtr == nullptr)
		return 0;

	while (startPtr < endPtr) {
		if (*startPtr == '\n') lineCount++;
		startPtr++;
	}
	m_totalFileLines = lineCount;
	return lineCount;
}


JsonSelectionNode* BTables::GetNextJsonSelection()
{
	if (!m_allSelectedJsonRows.size())
		return nullptr;
	JsonSelectionNode* j = m_allSelectedJsonRows.front();
	m_allSelectedJsonRows.pop();
	return j;
}