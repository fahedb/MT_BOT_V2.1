#include "BTables.h"
#include <iomanip>
#include <map>
#include <future>      // std::async, std::future

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

	std::vector<std::future<void>> futures; // Vector of cleaning tasks per thread

	for (int i = 0; i < m_allSelectedJsonRows_MultiThreaded.size(); i++) {
		futures.emplace_back(std::async(std::launch::async, [this, i]() {
			RemoveSelectionPerThread(i);
			}));
	}
	Logger::getInstance().WriteLog("Cleaning Json selection Started", false, true, LogType::LOG);
	for (auto& f : futures) f.get(); // Wait till all tasks finish
	Logger::getInstance().WriteLog("Cleaning Json selection Finished", false, true, LogType::LOG);
	m_allSelectedJsonRows_MultiThreaded.clear();
}

void BTables::RemoveSelectionPerThread(int threadIdx)
{
	std::vector<JsonSelectionNode*>& jsonRows = m_allSelectedJsonRows_MultiThreaded.at(threadIdx);
	int mySize = static_cast<int>(jsonRows.size());

	for (int t = 0; t < mySize; t++)
	{
		JsonSelectionNode* j = jsonRows[t];
		delete j;
	}
	jsonRows.clear();
}

bool BTables::SelectFromFile(int cols)
{
	std::ifstream file(m_fileName);

	if (!file.is_open()) {
		std::cerr << "Error opening file: " << m_fileName << std::endl;
		return false; // Return empty vector on error
	}

	std::string line;
	while (std::getline(file, line)) {
		std::vector<std::string> row;
		row = ProcessData::splitString(line, ",");
		if (row.size())
			m_allSelectedFileRows.push_back(new FileSelectionNode(row));
	}

	file.close();
	return true;
}
bool BTables::SelectFromJsonFile()
{
	if (m_multiThreadedRead)
		return SelectFromJsonFile_MultiThreaded();
	std::ifstream file(m_fileName);
	if (!file.is_open()) {
		std::cerr << "Error opening file: " << m_fileName << std::endl;
		return false; // Return empty vector on error
	}
	std::string line;
	while (std::getline(file, line)) {
		if (line.length() < 3)
			continue;
		m_allSelectedJsonRows.push(new JsonSelectionNode(nlohmann::json::parse(line)));
	}
	file.close();
	return true;
}

bool BTables::SelectFromJsonFile_MultiThreaded()
{
	Logger& logger = Logger::getInstance();
	
	logger.WriteLog("Counting Rows in DB Json file...", false, true);
	long linesCount = GetFileLinesCount(m_fileName);
	logger.WriteLog("Candles Found in Json DB file: " + std::to_string(linesCount),false,true);

	if (linesCount == 0) return false;

	int threadCount = std::thread::hardware_concurrency() - 1; // מספר תהליכונים
	logger.WriteLog("Threads Available for parallel read: " + std::to_string(threadCount),false,true);

	int chunkSize = (linesCount + threadCount - 1) / threadCount; // חלוקה מדויקת יותר
	logger.WriteLog("Every thread should load " + std::to_string(chunkSize) + " candles.",false,true);

	std::mutex loggerMutex, resultMutex;
	std::vector< std::vector<JsonSelectionNode*>> reads2D(threadCount);

	// Function to process a chunk of the loop
	auto processChunk = [&](int start, int end, int threadIndex) {

		{
			std::lock_guard<std::mutex> lock(loggerMutex);
			logger.WriteLog("Thread [" + std::to_string(threadIndex) + "] Started Pulling data...");
		}

		reads2D[threadIndex] = std::move(FileChunkRead(start, end, threadIndex, m_fileName)); // קריאה מקובץ

		{
			std::lock_guard<std::mutex> lock(loggerMutex);
			logger.WriteLog("Thread [" + std::to_string(threadIndex) + "] Done Pulling " + std::to_string(reads2D[threadIndex].size()) + " Candles." );
		}

	};

	// שימוש ב-std::async לניהול תהליכונים אוטומטי
	std::vector<std::future<void>> futures;
	for (int t = 0; t < threadCount; ++t) {
		int start = t * chunkSize;
		int end = (t == threadCount - 1) ? linesCount : (t + 1) * chunkSize;
		logger.WriteLog("Init new Thread for reading history, start:" + std::to_string(start) + " end: " + std::to_string(end));
		futures.emplace_back(std::async(std::launch::async, processChunk, start, end, t));
	}

	// שימוש ב-get() במקום join()
	for (auto& f : futures) {
		f.get(); // מחכה לסיום כל המשימות
	}

	// מיזוג הנתונים
	for (int i = 0; i < threadCount; i++)
	{
		std::vector<JsonSelectionNode*> vec = reads2D.at(i);
		std::vector<JsonSelectionNode*> localVec;
		for (int t = 0; t < vec.size(); t++)
		{
			//m_allSelectedJsonRows.push(vec.at(t));
			localVec.push_back(vec.at(t));
		}
		m_allSelectedJsonRows_MultiThreaded.push_back(localVec);
		m_jsonsThreadIdxsPlaces.push_back(0);
		reads2D.at(i).clear();
	}
	reads2D.clear();
	return true;
}

JsonSelectionNode* BTables::GetNextJsonSelection()
{
	if (!m_allSelectedJsonRows.size())
		return nullptr;
	JsonSelectionNode* j = m_allSelectedJsonRows.front();
	m_allSelectedJsonRows.pop();
	return j;
}

JsonSelectionNode* BTables::GetNextJsonSelection_MultiThreaded(int threadIdx)
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

long BTables::GetFileLinesCount(const std::string& filePath)
{
	// Open the file
	HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		std::cerr << "Error opening file: " << filePath << std::endl;
		return 0;
	}

	// Get file size (supporting files > 4GB)
	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize)) {
		std::cerr << "Error getting file size" << std::endl;
		CloseHandle(hFile);
		return 0;
	}

	// Create file mapping
	HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!hMapping) {
		std::cerr << "Error creating file mapping" << std::endl;
		CloseHandle(hFile);
		return 0;
	}

	// Map file to memory
	char* fileData = static_cast<char*>(MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0));
	if (!fileData) {
		std::cerr << "Error mapping file" << std::endl;
		CloseHandle(hMapping);
		CloseHandle(hFile);
		return 0;
	}

	// Count lines by iterating through file data
	long lineCount = 0;
	char* startPtr = fileData;
	char* endPtr = fileData + fileSize.QuadPart;

	while (startPtr < endPtr) {
		if (*startPtr == '\n') lineCount++;
		startPtr++;
	}

	// Clean up
	UnmapViewOfFile(fileData);
	CloseHandle(hMapping);
	CloseHandle(hFile);

	return lineCount;
}


std::vector<JsonSelectionNode*> BTables::FileChunkRead(int startLineIdx, int endLineIdx,int threadIdx,const std::string& fileName)
{
	std::this_thread::sleep_for(std::chrono::microseconds(15 * (threadIdx + 1)));
	std::vector<JsonSelectionNode*> vector;

	// Open the file
	HANDLE hFile = CreateFileA(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		std::cerr << "Error opening file: " << fileName << std::endl;
		CloseHandle(hFile);
		return vector;
	}

	// Create file Mapping
	HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!hMapping) {
		CloseHandle(hFile);
		std::cerr << "Error creating file mapping" << std::endl;
		return vector;
	}

	// Map to memory
	char* fileData = static_cast<char*>(MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0));
	if (!fileData) {
		CloseHandle(hMapping);
		CloseHandle(hFile);
		std::cerr << "Error mapping file" << std::endl;
		return vector;
	}

	// Size of file
	LARGE_INTEGER fileSize;

	if (!GetFileSizeEx(hFile, &fileSize)) {
		UnmapViewOfFile(fileData);
		CloseHandle(hMapping);
		CloseHandle(hFile);
		std::cerr << "Error getting file size" << std::endl;
		return vector;
	}

	char* startPtr = fileData;
	char* endPtr = fileData + fileSize.QuadPart;

	// Look for the relevant line
	int lineCount = 0;
	while (startPtr < endPtr && lineCount < startLineIdx) {
		if (*startPtr == '\n') lineCount++;
		startPtr++;
	}

	Logger& logger = Logger::getInstance();
	// read relevant lines
	logger.WriteLog("Trying to reserve " + std::to_string(endLineIdx - startLineIdx) + " cells for thread " + std::to_string(threadIdx) + " Starting from line: " + std::to_string(lineCount));
	vector.reserve(endLineIdx - startLineIdx);
	logger.WriteLog("Thread " + std::to_string(threadIdx) + " Started Collecting Candles...");
	std::string currentLine;
	currentLine.reserve(JSON_SLTRING_LEN);
	while (startPtr < endPtr && lineCount < endLineIdx) {
		if (*startPtr == '\n') {
			vector.push_back(new JsonSelectionNode(nlohmann::json::parse(currentLine)));
			currentLine.clear();
			lineCount++;
		}
		else {
			currentLine += *startPtr;
		}
		startPtr++;
	}
	logger.WriteLog("Thread " + std::to_string(threadIdx) + " Collected " + std::to_string(vector.size()) + " Candles.");

	// ניקוי ה-File Mapping
	UnmapViewOfFile(fileData);
	CloseHandle(hMapping);
	CloseHandle(hFile);

	return vector;
}