#pragma once
#include "..\MTSimulator_TimeLines\IndicatorEnums.h"

struct ValueTypeStream_t
{
	IndicatorValueType m_valueType;
	int m_numElements = 0; // Number of elements in the timeframe
	float* m_d_data = nullptr; // Array of pointers to device memory for each timeframe
};

struct TimeFrameStream_t
{
	BarPeriod m_barPeriod;
	int m_numValueTypes;
	ValueTypeStream_t* m_valueTypes; // Array of ValueTypeStream_t for each value type

};

struct LenIndicatorStream_t
{
	IndicatorLen m_indicatorType;
	int m_numTimeFrames;
	TimeFrameStream_t* m_timeFrames; // Array of TimeFramesStream_t for each timeframe
};

struct IndStream_t
{
	IndicatorType m_indicatorType;
	int m_numLens;
	LenIndicatorStream_t* m_lens; // Array of LenIndicatorStream_t for each length
};

struct dataStream_t
{
	int m_numIndicators = 0;
	IndStream_t* m_indicators; // Array of IndStream_t for each indicator

};
