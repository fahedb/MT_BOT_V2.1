#pragma once
#include "MTStreamsStructs.h"

class StreamSetters
{
public:
	static void CheckDataStreamTree(dataStream_t* dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT);
	static void CheckDataStream(dataStream_t* dataSt);
	static void CheckIndicatorStream(dataStream_t* dataSt, IndicatorType indT);
	static void CheckLenStream(dataStream_t* dataSt, IndicatorType indT, IndicatorLen indL);
	static void CheckTimeFrameStream(dataStream_t* dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP);
	static void CheckValueTypeStream(dataStream_t* dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT);

	static void ResetDataStreamTree_To0(dataStream_t* dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT);
	static void ResetDataStream_To0(dataStream_t* dataSt);
};

