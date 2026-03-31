#include "StreamSetters.h"
#include "CudaMemoryController.h"

#include <iostream>

IndicatorTypeList glob_indType_StreamSetters;
IndicatorLenList glob_indLen_StreamSetters;
BarPeriodList glob_barP_StreamSetters;
IndicatorValueTypeList glob_valType_StreamSetters;


void StreamSetters::ResetDataStreamTree_To0(dataStream_t* dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT, MemoryLocation memLocation) {

	int indTypeIndex = static_cast<int>(indT);
	int indLenIndex = static_cast<int>(indL);
	int barPeriodIndex = static_cast<int>(barP);
	int valTypeIndex = static_cast<int>(valT);

	float*& existingData = dataSt->m_indicators.at(indTypeIndex).m_lens.at(indLenIndex).m_timeFrames.at(barPeriodIndex).m_valueTypes.at(valTypeIndex).m_d_data;
	int numElements = dataSt->m_indicators.at(indTypeIndex).m_lens.at(indLenIndex).m_timeFrames.at(barPeriodIndex).m_valueTypes.at(valTypeIndex).m_numElements;

	if(!numElements || !existingData)
	{
		std::cout << "ResetDataStreamTree_To0: No elements to reset" << std::endl;
		return; // No elements to reset
	}
	switch (memLocation)
	{
	case MemoryLocation::HOST:
		memset(existingData, 0, sizeof(float) * numElements);
		break;
	case MemoryLocation::DEVICE:
		CudaMemorySet(existingData, 0, numElements);
		break;
	default:
		break;
	}
}

void StreamSetters::ResetDataStream_To0(dataStream_t* dataSt, MemoryLocation memLocation) // Reset all data streams to 0
{
	for(auto indT : glob_indType_StreamSetters.types)
	{
		for(auto indL : glob_indLen_StreamSetters.lengths)
		{
			for(auto barP : glob_barP_StreamSetters.periods)
			{
				for(auto valT : glob_valType_StreamSetters.valueTypes)
				{
					ResetDataStreamTree_To0(dataSt, indT, indL, barP, valT, memLocation);
				}
			}
		}
	}
}

void StreamSetters::AllocateDataStream(dataStream_t* dataSt, int numElements, MemoryLocation memLocation)
{
	for(auto indT : glob_indType_StreamSetters.types)
	{
		for(auto indL : glob_indLen_StreamSetters.lengths)
		{
			for(auto barP : glob_barP_StreamSetters.periods)
			{
				for(auto valT : glob_valType_StreamSetters.valueTypes)
				{
					AllocateDataStreamTree(dataSt, indT, indL, barP, valT, numElements, memLocation);
				}
			}
		}
	}
}

void StreamSetters::AllocateDataStreamTree(dataStream_t* dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT, int numElements, MemoryLocation memLocation)
{
	int indTypeIndex = static_cast<int>(indT);
	int indLenIndex = static_cast<int>(indL);
	int barPeriodIndex = static_cast<int>(barP);
	int valTypeIndex = static_cast<int>(valT);

	auto& inds = dataSt->m_indicators.at(indTypeIndex);
	auto& lens = inds.m_lens.at(indLenIndex);
	auto& timeFrames = lens.m_timeFrames.at(barPeriodIndex);
	float** existingData = &timeFrames.m_valueTypes.at(valTypeIndex).m_d_data;
	dataSt->m_indicators.at(indTypeIndex).m_lens.at(indLenIndex).m_timeFrames.at(barPeriodIndex).m_valueTypes.at(valTypeIndex).m_numElements = numElements;

	switch (memLocation)
	{
	case MemoryLocation::HOST:
		*existingData = new float[numElements]; // Allocate array for indicator values --HOST MEMORY
		break;
	case MemoryLocation::DEVICE:
		CudaMemoryAllocation(existingData, numElements); // Allocate array for indicator values --DEVICE MEMORY
		break;
	default:
		break;
	}
}

void StreamSetters::MemoryFreeDataStream(dataStream_t* dataSt, MemoryLocation memLocation)
{
	for(auto indT : glob_indType_StreamSetters.types)
	{
		for(auto indL : glob_indLen_StreamSetters.lengths)
		{
			for(auto barP : glob_barP_StreamSetters.periods)
			{
				for(auto valT : glob_valType_StreamSetters.valueTypes)
				{
					MemoryFreeDataStreamTree(dataSt, indT, indL, barP, valT, memLocation);
				}
			}
		}
	}
}

void StreamSetters::MemoryFreeDataStreamTree(dataStream_t* dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT, MemoryLocation memLocation)
{
	int indTypeIndex = static_cast<int>(indT);
	int indLenIndex = static_cast<int>(indL);
	int barPeriodIndex = static_cast<int>(barP);
	int valTypeIndex = static_cast<int>(valT);
	float*& existingData = dataSt->m_indicators.at(indTypeIndex).m_lens.at(indLenIndex).m_timeFrames.at(barPeriodIndex).m_valueTypes.at(valTypeIndex).m_d_data;
	if(!existingData)
	{
		return; // No data to free
	}

	switch (memLocation)
	{
	case MemoryLocation::HOST:
		delete[] existingData;
		existingData = nullptr;
		break;
	case MemoryLocation::DEVICE:
		CudaMemoryFree(existingData);
		existingData = nullptr;
		break;
	default:
		break;
	}
}

void StreamSetters::MoveDataToDevice(dataStream_t* hostDataSt, dataStream_t* deviceDataSt)
{
	for(auto indT : glob_indType_StreamSetters.types)
	{
		for(auto indL : glob_indLen_StreamSetters.lengths)
		{
			for(auto barP : glob_barP_StreamSetters.periods)
			{
				for(auto valT : glob_valType_StreamSetters.valueTypes)
				{
					int indTypeIndex = static_cast<int>(indT);
					int indLenIndex = static_cast<int>(indL);
					int barPeriodIndex = static_cast<int>(barP);
					int valTypeIndex = static_cast<int>(valT);
					float* hostData = hostDataSt->m_indicators.at(indTypeIndex).m_lens.at(indLenIndex).m_timeFrames.at(barPeriodIndex).m_valueTypes.at(valTypeIndex).m_d_data;
					int numElements = hostDataSt->m_indicators.at(indTypeIndex).m_lens.at(indLenIndex).m_timeFrames.at(barPeriodIndex).m_valueTypes.at(valTypeIndex).m_numElements;
					float* deviceDataPtr = deviceDataSt->m_indicators.at(indTypeIndex).m_lens.at(indLenIndex).m_timeFrames.at(barPeriodIndex).m_valueTypes.at(valTypeIndex).m_d_data;
					CudaMemoryCopyToDevice<float>(deviceDataPtr, hostData, numElements);
				}
			}
		}
	}
}

void StreamSetters::MoveDataToHost(dataStream_t* deviceDataSt, dataStream_t* hostDataSt)
{
	for(auto indT : glob_indType_StreamSetters.types)
	{
		for(auto indL : glob_indLen_StreamSetters.lengths)
		{
			for(auto barP : glob_barP_StreamSetters.periods)
			{
				for(auto valT : glob_valType_StreamSetters.valueTypes)
				{
					int indTypeIndex = static_cast<int>(indT);
					int indLenIndex = static_cast<int>(indL);
					int barPeriodIndex = static_cast<int>(barP);
					int valTypeIndex = static_cast<int>(valT);
					float* deviceData = deviceDataSt->m_indicators.at(indTypeIndex).m_lens.at(indLenIndex).m_timeFrames.at(barPeriodIndex).m_valueTypes.at(valTypeIndex).m_d_data;
					int numElements = deviceDataSt->m_indicators.at(indTypeIndex).m_lens.at(indLenIndex).m_timeFrames.at(barPeriodIndex).m_valueTypes.at(valTypeIndex).m_numElements;
					float* hostDataPtr = hostDataSt->m_indicators.at(indTypeIndex).m_lens.at(indLenIndex).m_timeFrames.at(barPeriodIndex).m_valueTypes.at(valTypeIndex).m_d_data;
					CudaMemoryCopyToHost<float>(hostDataPtr, deviceData, numElements);
				}
			}
		}
	}
}

void StreamSetters::MoveDataToDevice(const int* hostData, int** deviceData, int numElements)
{
	CudaMemoryCopyToDevice<int>(*deviceData, hostData, numElements);
}

void StreamSetters::MemoryAllocate(int** existingData, int numElements, MemoryLocation memLoc)
{
	switch (memLoc)
	{
	case MemoryLocation::HOST:
		*existingData = new int[numElements]; // Allocate array --HOST MEMORY
		break;
	case MemoryLocation::DEVICE:
		CudaMemoryAllocation(existingData, numElements); // Allocate array --DEVICE MEMORY
		break;
	default:
		break;
	}
}

void StreamSetters::MemoryReset(int* array, int count, MemoryLocation memLoc)
{
	switch (memLoc)
	{
	case MemoryLocation::HOST:
		memset(array, 0, sizeof(int) * count);
		break;
	case MemoryLocation::DEVICE:
		CudaMemorySet(array, 0, count);
		break;
	default:
		break;
	}
}
