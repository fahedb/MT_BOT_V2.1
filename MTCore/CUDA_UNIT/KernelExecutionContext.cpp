#include "KernelExecutionContext.h"
#include "StreamSetters.h"
#include "GPU_LaunchesParameters.h"
#include <iostream>
#include <chrono>

dataStream_t* KernelExecutionContext_PatternGod::Get_IndicatorsStreamDataPtr(DataTimeType timeType, MemoryLocation memLoc)
{
	if (memLoc == MemoryLocation::HOST || !m_availableDevice)
	{
		if (timeType == DataTimeType::HISTORIC)
			return &h_historyData;
		else // CURRENT
			return &h_currentData;
	}
	else // DEVICE
	{
		if (timeType == DataTimeType::HISTORIC)
			return &d_historyData;
		else // CURRENT
			return &d_currentData;
	}
}

dataStream_t* KernelExecutionContext_PatternGod::Get_ResultsDataStreamPtr(MemoryLocation memLoc)
{
	if (memLoc == MemoryLocation::HOST || !m_availableDevice)
		return &h_results;
	else // DEVICE
		return &d_results;
}

dataStream_t* KernelExecutionContext_PatternGod::Get_MinValuesStreamPtr(DataTimeType timeType, MemoryLocation memLoc)
{
	if(memLoc == MemoryLocation::HOST || !m_availableDevice)
	{
		if (timeType == DataTimeType::HISTORIC)
			return &h_histMinValues;
		else // CURRENT
			return &h_currMinValues;
	}
	else // DEVICE
	{
		if (timeType == DataTimeType::HISTORIC)
			return &d_histMinValues;
		else // CURRENT
			return &d_currMinValues;
	}
}

dataStream_t* KernelExecutionContext_PatternGod::Get_MaxValuesStreamPtr(DataTimeType timeType, MemoryLocation memLoc)
{
	if(memLoc == MemoryLocation::HOST || !m_availableDevice)
	{
		if (timeType == DataTimeType::HISTORIC)
			return &h_histMaxValues;
		else // CURRENT
			return &h_currMaxValues;
	}
	else // DEVICE
	{
		if (timeType == DataTimeType::HISTORIC)
			return &d_histMaxValues;
		else // CURRENT
			return &d_currMaxValues;
	}
}

KernelExecutionContext_PatternGod::KernelExecutionContext_PatternGod(int historySize, int currSize, bool availableDevice)
	: m_historySize(historySize), m_currentSize(currSize), m_availableDevice(availableDevice)
{
	Allocate();
}

void KernelExecutionContext_PatternGod::Allocate()
{
	h_totalScores = new float[m_historySize]; // Total scores size is based on history size
	totalScoresSize = m_historySize;

	StreamSetters::AllocateDataStream(&h_results, m_historySize, MemoryLocation::HOST); // Results size is based on history size
	
	StreamSetters::AllocateDataStream(&h_historyData, m_historySize, MemoryLocation::HOST); // History allocation -- Host side
	StreamSetters::AllocateDataStream(&h_histMinValues, m_historySize, MemoryLocation::HOST); // Historic min values allocation -- Host side
	StreamSetters::AllocateDataStream(&h_histMaxValues, m_historySize, MemoryLocation::HOST); // Historic max values allocation -- Host side

	StreamSetters::AllocateDataStream(&h_currentData, m_currentSize, MemoryLocation::HOST); // Current allocation -- Host side
	StreamSetters::AllocateDataStream(&h_currMinValues, 1 /*Need only one Min for pattern */, MemoryLocation::HOST); // Current min values allocation -- Host side
	StreamSetters::AllocateDataStream(&h_currMaxValues, 1 /*Need only one Max for pattern */, MemoryLocation::HOST); // Current max values allocation -- Host side

	if (m_availableDevice)
	{
		StreamSetters::AllocateDataStream(&d_historyData, m_historySize, MemoryLocation::DEVICE); // History allocation -- Device side
		StreamSetters::AllocateDataStream(&d_histMinValues, m_historySize, MemoryLocation::DEVICE); // Historic min values allocation -- Device side
		StreamSetters::AllocateDataStream(&d_histMaxValues, m_historySize, MemoryLocation::DEVICE); // Historic max values allocation -- Device side

		StreamSetters::AllocateDataStream(&d_currentData, m_currentSize, MemoryLocation::DEVICE); // Current allocation -- Device side
		StreamSetters::AllocateDataStream(&d_currMinValues, 1 /* Need only one min for pattern */, MemoryLocation::DEVICE); // Current min values allocation -- Device side
		StreamSetters::AllocateDataStream(&d_currMaxValues, 1 /* Need only one max for pattern */, MemoryLocation::DEVICE); // Current max values allocation -- Device side

		StreamSetters::AllocateDataStream(&d_results, m_historySize, MemoryLocation::DEVICE); // Results size is based on history size

	}
}

void KernelExecutionContext_PatternGod::PrepareForNewScan()
{
	// 1. Reset host results stream to 0
	StreamSetters::ResetDataStream_To0(&h_results, MemoryLocation::HOST);
	// 2. Reset host total results array to 0
	memset(h_totalScores, 0, sizeof(float) * totalScoresSize);

	if (m_availableDevice)
	{
		// 3. Reset device results stream to 0
		StreamSetters::ResetDataStream_To0(&d_results, MemoryLocation::DEVICE);
	}

}


void KernelExecutionContext_PatternGod::SwapDataStreams(dataStream_t& streamA, dataStream_t& streamB)
{
	auto temp = std::move(streamA);
	streamA = std::move(streamB);
	streamB = std::move(temp);
}

void KernelExecutionContext_PatternGod::SyncResultsToHost()
{
	StreamSetters::MoveDataToHost(&d_results, &h_results);
}

void KernelExecutionContext_PatternGod::Free()
{
	StreamSetters::MemoryFreeDataStream(&h_historyData, MemoryLocation::HOST);
	StreamSetters::MemoryFreeDataStream(&h_currentData, MemoryLocation::HOST);

	StreamSetters::MemoryFreeDataStream(&h_histMinValues, MemoryLocation::HOST);
	StreamSetters::MemoryFreeDataStream(&h_histMaxValues, MemoryLocation::HOST);
	StreamSetters::MemoryFreeDataStream(&h_currMinValues, MemoryLocation::HOST);
	StreamSetters::MemoryFreeDataStream(&h_currMaxValues, MemoryLocation::HOST);

	StreamSetters::MemoryFreeDataStream(&h_results, MemoryLocation::HOST);

	if (m_availableDevice)
	{
		StreamSetters::MemoryFreeDataStream(&d_historyData, MemoryLocation::DEVICE);
		StreamSetters::MemoryFreeDataStream(&d_currentData, MemoryLocation::DEVICE);
		StreamSetters::MemoryFreeDataStream(&d_histMinValues, MemoryLocation::DEVICE);
		StreamSetters::MemoryFreeDataStream(&d_histMaxValues, MemoryLocation::DEVICE);
		StreamSetters::MemoryFreeDataStream(&d_currMinValues, MemoryLocation::DEVICE);
		StreamSetters::MemoryFreeDataStream(&d_currMaxValues, MemoryLocation::DEVICE);

		StreamSetters::MemoryFreeDataStream(&d_results, MemoryLocation::DEVICE);
	}
}

void KernelExecutionContext_PatternGod::SyncHistoryDataToDevice(bool forceSync)
{
	if(!m_availableDevice)
	{
		std::cout << "KernelExecutionContext_PatternGod::SyncHistoryDataToDevice - No available device to sync to." << std::endl;
		return;
	}

	if (!m_isHistoryAlreadySyncedToDevice || forceSync)
	{
		StreamSetters::MoveDataToDevice(&h_historyData, &d_historyData);
		StreamSetters::MoveDataToDevice(&h_histMinValues, &d_histMinValues);
		StreamSetters::MoveDataToDevice(&h_histMaxValues, &d_histMaxValues);
		m_isHistoryAlreadySyncedToDevice = true;
	}
}

void KernelExecutionContext_PatternGod::SyncCurrentDataToDevice(bool forceSync)
{
	if(!m_availableDevice)
	{
		std::cout << "KernelExecutionContext_PatternGod::SyncCurrentDataToDevice - No available device to sync to." << std::endl;
		return;
	}

	if (!m_isThereANewCurrentData || forceSync)
	{
		StreamSetters::MoveDataToDevice(&h_currentData, &d_currentData);
		StreamSetters::MoveDataToDevice(&h_currMinValues, &d_currMinValues);
		StreamSetters::MoveDataToDevice(&h_currMaxValues, &d_currMaxValues);
		m_isThereANewCurrentData = false;
	}
}