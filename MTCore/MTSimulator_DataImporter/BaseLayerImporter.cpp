#include "BaseLayerImporter.h"
#include "ConfigsTableStaticProxy.h"
#include "ThreadPool.h"
#include "Logger.h"

#define MY_DATA_COUNT_EDGE 5000

template<typename QueueItemType>
QueueStruct<QueueItemType> BaseLayerImporter<QueueItemType>::Get_NextItem()
{
	int queueSize = 0;
	{
		std::lock_guard<std::mutex> lock(m_loadedDataQueue_mutex);
		queueSize = m_loadedDataQueue.size();
	}

	if (queueSize < MY_DATA_COUNT_EDGE)
	{
		std::queue<QueueStruct<QueueItemType>> newDataQueue;
		Init_DataSummaryChunk(newDataQueue, MY_DATA_COUNT_EDGE * 4); // Load a chunk of data
		Merge_LoadedData_FromQueue(newDataQueue);
	}
	std::lock_guard<std::mutex> lock(m_loadedDataQueue_mutex);

	if (m_loadedDataQueue.empty())
	{
		return QueueStruct<QueueItemType>(); // Return default constructed item if queue is empty
	}
	QueueStruct<QueueItemType> item = m_loadedDataQueue.front();
	m_loadedDataQueue.pop();
	return item;
}

template<typename QueueItemType>
BaseLayerImporter<QueueItemType>::~BaseLayerImporter()
{
	if (m_fileStartCharPtr)
	{
		UnmapViewOfFile(m_fileStartCharPtr);
	}
	if (m_hMapping)
	{
		CloseHandle(m_hMapping);
	}
	if (m_hFile)
	{
		CloseHandle(m_hFile);
	}
}

template<typename QueueItemType>
void BaseLayerImporter<QueueItemType>::InitMyImporter()
{
	if (!MapMyFile())
	{
		Logger::getInstance().WriteLog("BaseLayerImporter::InitMyImporter - Could not map file: " + m_filePath, false, true, LogType::ERR);
		return;
	}
	m_totalFileLines = GetFileLinesCount();
	m_fileCurrPosition = m_policiesDataStartPtr;
}

template<typename QueueItemType>
std::string BaseLayerImporter<QueueItemType>::GetNextDataSummaryLine()
{
	if (!m_fileStartCharPtr || !m_fileCurrPosition)
	{
		Logger::getInstance().WriteLog("BaseLayerImporter::GetNextDataSummaryLine - File not mapped properly", false, true, LogType::ERR);
		return "";
	}
	char* endPtr = m_dataSummaryEndPtr;
	char* currPtr = m_fileCurrPosition;
	std::string line;
	while (currPtr < endPtr) {
		if (*currPtr == '\n') {
			UpdateCurrLine(m_currentLine + 1); // Update current line index
			currPtr++; // Move past the newline character
			break;
		}
		line += *currPtr;
		currPtr++;
	}
	m_fileCurrPosition = currPtr; // Update current position
	return line;
}

template<typename QueueItemType>
std::string BaseLayerImporter<QueueItemType>::GetNextPoliciesDataLine()
{
	if (!m_fileStartCharPtr || !m_fileCurrPosition)
	{
		Logger::getInstance().WriteLog("BaseLayerImporter::GetNextPoliciesDataLine - File not mapped properly", false, true, LogType::ERR);
		return "";
	}
	char* endPtr = m_policiesDataEndPtr;
	char* currPtr = m_fileCurrPosition;
	std::string line;
	while (currPtr < endPtr) {
		if (*currPtr == '\n') {
			currPtr++; // Move past the newline character
			break;
		}
		line += *currPtr;
		currPtr++;
	}
	m_fileCurrPosition = currPtr; // Update current position
	return line;
}

template<typename QueueItemType>
void BaseLayerImporter<QueueItemType>::Init_DataSummaryChunk(std::queue<QueueStruct<QueueItemType>>& outData, int chunkSize)
{
	int counter = 0;
	std::string line = GetNextDataSummaryLine();
	while(!line.empty() && counter < chunkSize)
	{
		QueueStruct<QueueItemType> item = Create_QueueItem(line, m_currentLine);
		outData.push(item); // Add item to output queue
		counter++; // Increment counter
		line = GetNextDataSummaryLine();
	}
}

template<typename QueueItemType>
void BaseLayerImporter<QueueItemType>::Init_bktstPolicies()
{
	std::string line = GetNextPoliciesDataLine();
	// Example for config in bktst file: SOFTMAX_T_PARAM_FOR_VOTING=0.5
	// Need to break the line into key and value
	while (!line.empty())
	{
		size_t delimiterPos = line.find('=');
		if (delimiterPos != std::string::npos)
		{
			std::string key = line.substr(0, delimiterPos);
			std::string value = line.substr(delimiterPos + 1);
			ConfigsTableStaticProxy::Insert_IntoForcedConfigs_DB(key, value);
		}
		line = GetNextPoliciesDataLine();
	}
}

template<typename QueueItemType>
void BaseLayerImporter<QueueItemType>::Insert_LoadedData_ToQueue(const QueueStruct<QueueItemType>& item)
{
	std::lock_guard<std::mutex> lock(m_loadedDataQueue_mutex);
	m_loadedDataQueue.push(item);
}

template<typename QueueItemType>
void BaseLayerImporter<QueueItemType>::Merge_LoadedData_FromQueue(std::queue<QueueStruct<QueueItemType>>& inData)
{
	std::lock_guard<std::mutex> lock(m_loadedDataQueue_mutex);
	int counter = 0;
	while (inData.size())
	{
		m_loadedDataQueue.push(inData.front());
		inData.pop();
		counter++;
	}

	std::lock_guard<std::mutex> maxLock(m_maxLoadedLineIndex_mutex);
	m_maxLoadedLineIndex += counter;
}

template<typename QueueItemType>
ChangeNode* BaseLayerImporter<QueueItemType>::Get_ChangeNode(long long dateTime)
{
	int idx = m_historyDBMap->GetIdx_Of_TimePoint(dateTime);
	if (idx >= 0 && idx < m_historyTL->size())
	{
		return (*m_historyTL)[idx];
	}
	return nullptr;
}

template<typename QueueItemType>
bool BaseLayerImporter<QueueItemType>::MapMyFile()
{
	// Open the file
	HANDLE hFile = CreateFileA(m_filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		Logger::getInstance().WriteLog("Error opening file: " + m_filePath, false, true, LogType::ERR);
		CloseHandle(hFile);
		return false;
	}

	// Create file Mapping
	HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!hMapping) {
		CloseHandle(hFile);
		Logger::getInstance().WriteLog("Error opening file: " + m_filePath, false, true, LogType::ERR);
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

	m_fileCurrPosition = fileData; // Initialize current position to start of file

	m_fileStartCharPtr = fileData; // Start of file
	m_fileEndCharPtr = fileData + fileSize.QuadPart; // End of file

	m_policiesDataStartPtr = GetBeginGap(PoliciesDataKey); // Get pointer to policies data start
	m_policiesDataEndPtr = GetBeginGap(DataSummaryKey) - std::strlen(DataSummaryKey); // Get pointer to policies data end

	m_dataSummaryStartPtr = m_policiesDataEndPtr + std::strlen(DataSummaryKey); // Get pointer to data summary start
	m_dataSummaryEndPtr = m_fileEndCharPtr; // End of file
	return true;
}

template<typename QueueItemType>
char* BaseLayerImporter<QueueItemType>::GetBeginGap(const std::string& endOfThisStr)
{
	char* startPtr = m_fileStartCharPtr;
	char* endPtr = m_fileEndCharPtr;
	if (startPtr == nullptr)
	{
		Logger::getInstance().WriteLog("BaseLayerImporter::GetBeginGap - File not mapped properly", false, true, LogType::ERR);
		return nullptr;
	}size_t strLen = endOfThisStr.length();
	char* currPtr = startPtr;
	bool found = false;
	while (currPtr < endPtr - strLen) {
		if (strncmp(currPtr, endOfThisStr.c_str(), strLen) == 0) {
			currPtr += strLen;
			found = true;
			break;
		}
		currPtr++;
	}
	if(!found)
	{
		Logger::getInstance().WriteLog("BaseLayerImporter::GetBeginGap - Could not find the specified string: " + endOfThisStr, false, true, LogType::ERR);
		return nullptr;
	}
	return currPtr;
	
}

template<typename QueueItemType>
long BaseLayerImporter<QueueItemType>::GetFileLinesCount()
{
	if (m_totalFileLines > 0)
		return m_totalFileLines;
	// Count lines by iterating through file data
	long lineCount = 0;
	char* startPtr = m_fileStartCharPtr;
	char* endPtr = m_fileEndCharPtr;
	if (startPtr == nullptr)
		return 0;

	while (startPtr < endPtr) {
		if (*startPtr == '\n') lineCount++;
		startPtr++;
	}
	m_totalFileLines = lineCount;
	return lineCount;
}


