#pragma once
#include "MTStreamsStructs.h"

class StreamSetters
{
public:
	static void ResetDataStreamTree_To0(dataStream_t* dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT, MemoryLocation memLocation);
	static void ResetDataStream_To0(dataStream_t* dataSt, MemoryLocation memLocation);

	static void AllocateDataStream(dataStream_t* dataSt, int numElements, MemoryLocation memLocation);
	static void AllocateDataStreamTree(dataStream_t* dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT, int numElements, MemoryLocation memLocation);

	static void MemoryFreeDataStream(dataStream_t* dataSt, MemoryLocation memLocation);
	static void MemoryFreeDataStreamTree(dataStream_t* dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT, MemoryLocation memLocation);

	static void MoveDataToDevice(dataStream_t* hostDataSt, dataStream_t* deviceDataSt);
	static void MoveDataToHost(dataStream_t* deviceDataSt, dataStream_t* hostDataSt);

	static void MoveDataToDevice(const int* hostData, int** deviceData, int numElements);

	static void MemoryAllocate(int** existingData, int numElements, MemoryLocation memLoc);

	static void MemoryReset(int* array, int count, MemoryLocation memLoc);
};

// --------------------------------------------------------------------------------------------------------------	
// --------------------------------------------------------------------------------------------------------------	



