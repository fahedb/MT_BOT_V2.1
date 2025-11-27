#pragma once
#include "MTStreamsStructs.h"

class KernelExecutionContext_PatternGod
{
public:
	// Getters
	float* Get_HostTotalScoresPtr() { return h_totalScores; }
	int* Get_AfterFilterIndicesPtr(MemoryLocation memLoc);
	int Get_AfterFilterIndicesCount() { return afterFilterIndicesCount; }

	void Import_AfterFilterIndices(std::vector<int> inIndices);

	dataStream_t* Get_ResultsDataStreamPtr(MemoryLocation memLoc); // Host or Device

	dataStream_t* Get_IndicatorsStreamDataPtr(DataTimeType timeType, MemoryLocation memLoc); // Get indicator data stream --Host or Device
	dataStream_t* Get_MinValuesStreamPtr(DataTimeType timeType, MemoryLocation memLoc); // Get Min values -- Host or Device
	dataStream_t* Get_MaxValuesStreamPtr(DataTimeType timeType, MemoryLocation memLoc); // Get max values -- Host or Device
	
private:
	// --- HOST SIDE ---
	int* h_afterFilterIndices; // Host after filter indices array
	int afterFilterIndicesCount;
	float* h_totalScores; // Host total scores array
	dataStream_t h_results; // Host results data stream 
	
	dataStream_t h_historyData; // Historic data stream
	dataStream_t h_histMinValues; // Historic min values stream
	dataStream_t h_histMaxValues; // Historic max values stream

	dataStream_t h_currentData; // Current data stream
	dataStream_t h_currMinValues; // Current min values stream
	dataStream_t h_currMaxValues; // Current max values stream

	// --- DEVICE SIDE ---
	int* d_afterFilterIndices; // Device after filter indices array
	dataStream_t d_historyData; // Historic data stream
	dataStream_t d_currentData; // Current data stream

	dataStream_t d_results; // Current scan results

	dataStream_t d_histMinValues;
	dataStream_t d_histMaxValues;

	dataStream_t d_currMinValues;
	dataStream_t d_currMaxValues;

private:
	int contextId;

	int m_historySize;
	int m_currentSize;

	bool m_isHistoryAlreadySyncedToDevice = false;
	bool m_isThereANewCurrentData = false;
public:
	KernelExecutionContext_PatternGod(int historySize, int currSize);
	~KernelExecutionContext_PatternGod() { Free(); };

	void PrepareForNewScan();
	void SyncResultsToHost();

	void SyncHistoryDataToDevice(bool forceSync = false);
	void SyncCurrentDataToDevice(bool forceSync = false);

private:
	void Free();
	void Allocate();

private:
	// Static helper functions:

	static void SwapDataStreams(dataStream_t& streamA, dataStream_t& streamB);
};
