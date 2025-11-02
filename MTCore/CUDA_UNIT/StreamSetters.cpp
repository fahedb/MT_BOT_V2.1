#include "StreamSetters.h"
#include <iostream>

IndicatorTypeList glob_indType_StreamSetters;
IndicatorLenList glob_indLen_StreamSetters;
BarPeriodList glob_barP_StreamSetters;
IndicatorValueTypeList glob_valType_StreamSetters;

void StreamSetters::ResetDataStream_To0(dataStream_t** dataSt) // Reset all data streams to 0
{
	for(auto indT : glob_indType_StreamSetters.types)
	{
		for(auto indL : glob_indLen_StreamSetters.lengths)
		{
			for(auto barP : glob_barP_StreamSetters.periods)
			{
				for(auto valT : glob_valType_StreamSetters.valueTypes)
				{
					ResetDataStreamTree_To0(dataSt, indT, indL, barP, valT);
				}
			}
		}
	}
}

void StreamSetters::ResetDataStreamTree_To0(dataStream_t** dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT) {
	
	CheckDataStreamTree(dataSt, indT, indL, barP, valT); // Ensure the data stream tree is properly initialized
	float*& existingData = (*dataSt)->m_indicators[static_cast<int>(indT)]
		.m_lens[static_cast<int>(indL)].m_timeFrames[static_cast<int>(barP)].m_valueTypes[static_cast<int>(valT)].m_d_data;
	int numElements = (*dataSt)->m_indicators[static_cast<int>(indT)]
		.m_lens[static_cast<int>(indL)].m_timeFrames[static_cast<int>(barP)].m_valueTypes[static_cast<int>(valT)].m_numElements;
	if(!numElements || !existingData)
	{
		std::cout << "ResetDataStreamTree_To0: No elements to reset" << std::endl;
		return; // No elements to reset
	}
	
	memset(existingData, 0,
					sizeof(float) * (*dataSt)->m_indicators[static_cast<int>(indT)]
					.m_lens[static_cast<int>(indL)].m_timeFrames[static_cast<int>(barP)].m_valueTypes[static_cast<int>(valT)].m_numElements);
}

void StreamSetters::CheckDataStreamTree(dataStream_t** dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT)
{
	CheckDataStream(dataSt); // Initialize if null
	CheckIndicatorStream(dataSt, indT); // Ensure indicator type exists
	CheckLenStream(dataSt, indT, indL); // Ensure indicator length exists
	CheckTimeFrameStream(dataSt, indT, indL, barP); // Ensure timeframe exists
	CheckValueTypeStream(dataSt, indT, indL, barP, valT); // Ensure value type exists
}

void StreamSetters::CheckDataStream(dataStream_t** dataSt)
{
	if (*dataSt == nullptr)
	{
		*dataSt = new dataStream_t();
	}
}
void StreamSetters::CheckIndicatorStream(dataStream_t** dataSt, IndicatorType indT)
{
	int indTypeIndex = static_cast<int>(indT);
	while ((*dataSt)->m_indicators.size() <= indTypeIndex)
	{
		(*dataSt)->m_indicators.push_back(IndStream_t());
	}
	(*dataSt)->m_indicators[indTypeIndex].m_indicatorType = indT;
}

void StreamSetters::CheckLenStream(dataStream_t** dataSt, IndicatorType indT, IndicatorLen indL)
{
	int indicatorLenIndex = static_cast<int>(indL);
	int indTypeIndex = static_cast<int>(indT);
	while ((*dataSt)->m_indicators[indTypeIndex].m_lens.size() <= indicatorLenIndex)
	{
		(*dataSt)->m_indicators[indTypeIndex].m_lens.push_back(LenIndicatorStream_t());
	}
	(*dataSt)->m_indicators[indTypeIndex].m_lens[indicatorLenIndex].m_indicatorType = indL;
}

void StreamSetters::CheckTimeFrameStream(dataStream_t** dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP)
{
	int indTypeIndex = static_cast<int>(indT);
	int indicatorLenIndex = static_cast<int>(indL);
	int timeFrameIndex = static_cast<int>(barP);

	while ((*dataSt)->m_indicators[indTypeIndex].m_lens[indicatorLenIndex].m_timeFrames.size() <= timeFrameIndex)
	{
		(*dataSt)->m_indicators[indTypeIndex].m_lens[indicatorLenIndex].m_timeFrames.push_back(TimeFrameStream_t());
	}
	(*dataSt)->m_indicators[indTypeIndex].m_lens[indicatorLenIndex].m_timeFrames[timeFrameIndex].m_barPeriod = barP;
}

void StreamSetters::CheckValueTypeStream(dataStream_t** dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT)
{
	int indTypeIndex = static_cast<int>(indT);
	int indicatorLenIndex = static_cast<int>(indL);
	int timeFrameIndex = static_cast<int>(barP);
	int valueTypeIndex = static_cast<int>(valT);
	// Expand vector until it's large enough to safely access valueTypeIndex
	while ((*dataSt)->m_indicators[indTypeIndex].m_lens[indicatorLenIndex].m_timeFrames[timeFrameIndex].m_valueTypes.size() <= valueTypeIndex)
	{
		(*dataSt)->m_indicators[indTypeIndex].m_lens[indicatorLenIndex].m_timeFrames[timeFrameIndex].m_valueTypes.push_back(ValueTypeStream_t());
	}
	(*dataSt)->m_indicators[indTypeIndex].m_lens[indicatorLenIndex].m_timeFrames[timeFrameIndex].m_valueTypes[valueTypeIndex].m_valueType = valT;
}

