#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "QueueStruct.h"

#include <string>
#include <windows.h>
#include <mutex>
#include <queue>

#include "TimeLine.h"
#include "..\MTSimulator_DataExporter\LayersJsonKeys.h"
template<typename QueueItemType>
class BaseLayerImporter
{
public:
	BaseLayerImporter(const std::string& filePath, std::vector<ChangeNode*>* historyTL, DBMap* historyDBMap) : m_filePath(filePath), m_totalFileLines(0), m_currentLine(-1), m_maxLoadedLineIndex(-1), m_historyTL(historyTL), m_historyDBMap(historyDBMap) {}
	~BaseLayerImporter();

	void InitMyImporter(); // Need to call this before using the importer
	QueueStruct<QueueItemType> Get_NextItem(); // Get the next item from the loaded data queue
protected:
	virtual QueueStruct<QueueItemType> Create_QueueItem(const std::string& line, long lineIndex) = 0; // Pure virtual function to create a QueueStruct item from a line

	std::string GetNextPoliciesDataLine();
	std::string GetNextDataSummaryLine();

	void Init_DataSummaryChunk(std::queue<QueueStruct<QueueItemType>>& outData, int chunkSize);

	void Init_bktstPolicies();
	void Insert_LoadedData_ToQueue(const QueueStruct<QueueItemType>& item);
	void Merge_LoadedData_FromQueue(std::queue<QueueStruct<QueueItemType>>& inData);

	ChangeNode* Get_ChangeNode(long long dateTime);

private:
	std::vector<ChangeNode*>* m_historyTL;
	DBMap* m_historyDBMap;

	std::string m_filePath;

	std::mutex m_loadedDataQueue_mutex;
	
	std::queue<QueueStruct<QueueItemType>> m_loadedDataQueue; // Queue to hold loaded data items

	std::mutex m_currentLine_mutex; // Mutex for thread-safe access to m_currentLine
	long m_currentLine; // The current line index being processed
	long m_totalFileLines; // Total number of lines in the file

	std::mutex m_maxLoadedLineIndex_mutex; // Mutex for thread-safe access to m_maxLoadedLineIndex
	long m_maxLoadedLineIndex; // The maximum line index that has been loaded so far

	char* m_fileCurrPosition = nullptr; // Pointer to current position in the file mapping

	char* m_fileStartCharPtr = nullptr; // Pointer to the start of the file mapping
	char* m_fileEndCharPtr = nullptr; // Pointer to the end of the file mapping

	char* m_policiesDataStartPtr = nullptr; // Pointer to the start of the policies data section
	char* m_policiesDataEndPtr = nullptr; // Pointer to the end of the policies data section

	char* m_dataSummaryStartPtr = nullptr; // Pointer to the start of the data summary section
	char* m_dataSummaryEndPtr = nullptr; // Pointer to the end of the data summary section

	HANDLE m_hFile = nullptr;
	HANDLE m_hMapping = nullptr;

	bool MapMyFile(); // Map the file into memory
	char* GetBeginGap(const std::string& endOfThisStr); // Get the pointer to the end of the specified string
	long GetFileLinesCount(); // Get the total number of lines in the file
	void UpdateCurrLine(long lineIdx) {
		std::lock_guard<std::mutex> lock(m_currentLine_mutex);
		m_currentLine = lineIdx;
	}

};
