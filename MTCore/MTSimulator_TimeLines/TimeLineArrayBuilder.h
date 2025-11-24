#pragma once
#include "ChangeData.h"
#include "MTStreamsStructs.h"

class TimeLineArrayBuilder
{
	static void BuildMinMaxArray(const float* inData, float* outMinData, float* outMaxData, int resNum, int patternLen);
	static void BuildTimeLineArray(float* outData, std::vector<ChangeNode*>* tl, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT);;
public:
	static void BuildTimeLineArrays(dataStream_t* outData, std::vector<ChangeNode*>* tl);
	static void BuildMinMaxArrays(dataStream_t* inData, dataStream_t* outMinData, dataStream_t* outMaxData, int resNum, int patternLen);

	static void BuildTotalScoreArray(dataStream_t* inData, float* outData, int resNum);
	static void BuildTotalScoreArray_ChunkTask(dataStream_t* inData, float* outData, int resNum, int threadIdx, int chunkSize);
	static void BuildTotalScoreArray_SinglePattern(TimeFrameStream_t& timeFrameStream, float* outData, int resNum, int threadIdx, int chunkSize);
};