#include "CudaEngine_ScoreCalcs.h"
#include "Logger.h"
#include "CPP_To_Cuda_Tunnel.h"
#include "StreamSetters.h"

IndicatorTypeList glob_indType_CudaEngine;
IndicatorLenList glob_indLen_CudaEngine;
BarPeriodList glob_barP_CudaEngine;
IndicatorValueTypeList glob_valType_CudaEngine;

#define COMAPRE_WEIGHT 10



void CudaEngine_ScoreCalcs::DataStreamSetup(std::vector<ChangeNode*>* tl, dataStream_t* dataSt, bool forceOverWrite)
{
	if (!tl)
	{
		Logger::getInstance().WriteLog("CudaEngine_ScoreCalcs::DataStreamSetup - Invalid data pointer", false, true, LogType::ERR);
		return;
	}

	for (auto indT : glob_indType_CudaEngine.types)
	{
		for (auto indL : glob_indLen_CudaEngine.lengths)
		{
			for (auto barP : glob_barP_CudaEngine.periods)
			{
				for (auto valT : glob_valType_CudaEngine.valueTypes)
				{
					ValueTypeStream_t* valStream = &dataSt->m_indicators[static_cast<int>(indT)].m_lens[static_cast<int>(indL)].m_timeFrames[static_cast<int>(barP)].m_valueTypes[static_cast<int>(valT)];
					if(valStream->m_d_data && !forceOverWrite)
					{
						continue; // Data already exists, do not overwrite
					}
					if(valStream->m_d_data && forceOverWrite)
					{
						delete[] valStream->m_d_data; // Free existing memory before overwriting
						valStream->m_d_data = nullptr;
						valStream->m_numElements = 0;
					}

					float* dataArray = new float[tl->size()]; // Allocate array for indicator values
					for (size_t i = 0; i < tl->size(); i++)
					{
						ChangeNode* node = tl->at(i);
						if (node)
						{
							BaseIndicator* indicator = node->Get_Indicator_Pointer(indT, indL);
							if (indicator)
							{
								double value = indicator->Get_IndicatorValues_Pointer()->Get_Value(valT, barP);
								dataArray[i] = static_cast<float>(value);
							}
							else
							{
								dataArray[i] = -1.0f; // Default value if indicator is missing
							}
						}
						else
						{
							dataArray[i] = -1.0f; // Default value if node is null
						}
					}
					valStream->m_numElements = tl->size();
					valStream->m_d_data = dataArray;
					CPP_To_Cuda_Tunnel::Set_CurrDataTimeFreameStream(valStream->m_d_data, valStream->m_numElements, indT, indL, barP, valT, forceOverWrite);
				}
			}
		}
	}
}

void CudaEngine_ScoreCalcs::ResultStreamSetup(dataStream_t* dataSt, int resSize, bool forceOverWrite)
{

	for (auto indT : glob_indType_CudaEngine.types)
	{
		for (auto indL : glob_indLen_CudaEngine.lengths)
		{
			for (auto barP : glob_barP_CudaEngine.periods)
			{
				for (auto valT : glob_valType_CudaEngine.valueTypes)
				{
					ValueTypeStream_t* valStream = &dataSt->m_indicators[static_cast<int>(indT)].m_lens[static_cast<int>(indL)].m_timeFrames[static_cast<int>(barP)].m_valueTypes[static_cast<int>(valT)];
					if (valStream->m_d_data && !forceOverWrite)
					{
						continue; // Data already exists, do not overwrite
					}
					if (valStream->m_d_data && forceOverWrite)
					{
						delete[] valStream->m_d_data; // Free existing memory before overwriting
						valStream->m_d_data = nullptr;
						valStream->m_numElements = 0;
					}

					float* dataArray = new float[resSize]; // Allocate array for indicator values
					
					valStream->m_numElements = resSize;
					valStream->m_d_data = dataArray;
					memset(valStream->m_d_data, 0, sizeof(float) * resSize); // Initialize to zero
					CPP_To_Cuda_Tunnel::Set_ResultsTimeFrameStream(valStream->m_numElements, indT, indL, barP, valT, forceOverWrite);
				}
			}
		}
	}
}

void CudaEngine_ScoreCalcs::ResetStreamToZero(dataStream_t* dataSt)
{
	StreamSetters::ResetDataStream_To0(dataSt);

	for(auto indT : glob_indType_CudaEngine.types)
	{
		for(auto indL : glob_indLen_CudaEngine.lengths)
		{
			for(auto barP : glob_barP_CudaEngine.periods)
			{
				for(auto valT : glob_valType_CudaEngine.valueTypes)
				{
					CPP_To_Cuda_Tunnel::Reset_ResultsStream_To0(indT, indL, barP, valT);
				}
			}
		}
	}

	memset(m_h_totalScores, 0, sizeof(float) * m_currentData->size());

}

void CudaEngine_ScoreCalcs::Initialize()
{
	if (!m_currentData || !m_historicData)
	{
		Logger::getInstance().WriteLog("CudaEngine_ScoreCalcs::Initialize - Invalid data pointers", false, true, LogType::ERR);
		return;
	}
	if (!m_h_totalScores)
		m_h_totalScores = new float[m_currentData->size()];
	ResultStreamSetup(m_h_results, m_currentData->size(), false); // Setup results stream without overwriting existing data
	DataStreamSetup(m_historicData, m_h_histDataStream, false); // Setup historic data stream without overwriting existing data
	DataStreamSetup(m_currentData, m_h_currDataStream, true); // Setup current data stream with overwriting existing data
}


void CudaEngine_ScoreCalcs::RunCudaScan(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D, int scoreToAct, int numThreads)
{
	ResetStreamToZero(m_h_results); // Reset results stream to zero before running scan
	for (auto indT : glob_indType_CudaEngine.types)
	{
		RunScan_PerIndicator(indT);
	}

	for (auto indT : glob_indType_CudaEngine.types)
	{
		RunScan_TwoIndicatorsRatio(indT);
	}
	// TODO: Collect results from m_h_totalScores and populate resultNodes2D
	int limit = static_cast<int>(m_historicData->size() - m_currentData->size());

	for (size_t i = 0; i < limit; i++)
	{
		if (m_h_totalScores[i] >= scoreToAct)
		{
			ScoresResult scoreResult = CombineAllResults_ForIdx(i, m_h_results);
			scoreResult.calcTotalResult();
			resultNodes2D.push_back(std::make_pair(m_historicData->at(i + m_currentData->size() - 1), scoreResult));
		}
	}

}

void CudaEngine_ScoreCalcs::Set_NewCurrentTimeLine(std::vector<ChangeNode*>* currentData)
{
	m_currentData = currentData;
	DataStreamSetup(m_currentData, m_h_currDataStream, true); // Setup current data stream with overwriting existing data

}

void CudaEngine_ScoreCalcs::Set_NewHistoricTimeLine(std::vector<ChangeNode*>* historicData)
{
	m_historicData = historicData;
	DataStreamSetup(m_historicData, m_h_histDataStream, true); // Setup historic data stream with overwriting existing data
}

void CudaEngine_ScoreCalcs::RunScan_PerIndicator(IndicatorType indT)
{
	for(auto indL : glob_indLen_CudaEngine.lengths)
	{
		RunScan_PerIndicator_IndLen(indT, indL);
	}
}

void CudaEngine_ScoreCalcs::RunScan_PerIndicator_IndLen(IndicatorType indT, IndicatorLen indL)
{
	// Implementation for scanning per indicator and length
	for(auto barP : glob_barP_CudaEngine.periods)
	{
		RunScan_PerIndicator_BarPeriod(indT, indL, barP);
	}
}

void CudaEngine_ScoreCalcs::RunScan_PerIndicator_BarPeriod(IndicatorType indT, IndicatorLen indL, BarPeriod barP)
{
	// Implementation for scanning per indicator, length, and bar period
	float* resData= m_h_results->m_indicators[static_cast<int>(indT)]
		.m_lens[static_cast<int>(indL)]
		.m_timeFrames[static_cast<int>(barP)]
		.m_valueTypes[static_cast<int>(IndicatorValueType::Value)].m_d_data;
	int numResults = m_h_results->m_indicators[static_cast<int>(indT)]
		.m_lens[static_cast<int>(indL)]
		.m_timeFrames[static_cast<int>(barP)]
		.m_valueTypes[static_cast<int>(IndicatorValueType::Value)].m_numElements;

	CPP_To_Cuda_Tunnel::Calc_Indicator_ScoreCalcs(resData, numResults, indT, indL, barP, COMAPRE_WEIGHT);
	for(int i =0 ; i < numResults; i++)
	{
		m_h_totalScores[i] += resData[i];
	}
}

void CudaEngine_ScoreCalcs::RunScan_TwoIndicatorsRatio(IndicatorType indT)
{
	// Meed to scan for every LenX and LenY --NOTE Scanned(LenX,LenY) == Scanned(LenY,LenX) so we can skip half of the scans
	std::vector<std::pair<IndicatorLen, IndicatorLen>> scannedLens;
	for(auto lenX : glob_indLen_CudaEngine.lengths)
	{
		for(auto lenY : glob_indLen_CudaEngine.lengths)
		{
			if (AreLenPairsScanned(scannedLens, lenX, lenY))
			{
				RunScan_TwoIndicatorsRatio_IndLens(indT, lenX, lenY);
			}
		}
	}
}

void CudaEngine_ScoreCalcs::RunScan_TwoIndicatorsRatio_IndLens(IndicatorType indT, IndicatorLen lenX, IndicatorLen LenY)
{
	std::vector<std::pair<BarPeriod, BarPeriod>> scannedBarPs;
	for(auto barPX : glob_barP_CudaEngine.periods)
	{
		for(auto barPY : glob_barP_CudaEngine.periods)
		{
			if (AreBarPeriodPairsScanned(scannedBarPs, barPX, barPY))
			{
				RunScan_TwoIndicatorsRatio_BarPeriods(indT, lenX, barPX, LenY, barPY);
			}
		}
	}
}

void CudaEngine_ScoreCalcs::RunScan_TwoIndicatorsRatio_BarPeriods(IndicatorType indT, IndicatorLen lenX, BarPeriod barPX, IndicatorLen LenY, BarPeriod barPY)
{
	if(lenX == LenY &&  barPX == barPY)
	{
		return; // Skip same length and same bar period
	}
	float* resData= m_h_results->m_indicators[static_cast<int>(indT)]
		.m_lens[static_cast<int>(lenX)]
		.m_timeFrames[static_cast<int>(barPX)]
		.m_valueTypes[static_cast<int>(IndicatorValueType::Value)].m_d_data;

	int numResults = m_h_results->m_indicators[static_cast<int>(indT)]
		.m_lens[static_cast<int>(lenX)]
		.m_timeFrames[static_cast<int>(barPX)]
		.m_valueTypes[static_cast<int>(IndicatorValueType::Value)].m_numElements;
	
	CPP_To_Cuda_Tunnel::Calc_TwoIndicatorsRatio_ScoreCalcs(resData, numResults, indT, lenX, barPX, LenY, barPY, COMAPRE_WEIGHT);

	for(int i =0 ; i < numResults; i++)
	{
		m_h_totalScores[i] += resData[i];
	}
}

bool CudaEngine_ScoreCalcs::AreLenPairsScanned(const std::vector<std::pair<IndicatorLen, IndicatorLen>>& scannedLens, IndicatorLen lenX, IndicatorLen lenY)
{
	for(const auto& pair : scannedLens)
	{
		if((pair.first == lenX && pair.second == lenY) || (pair.first == lenY && pair.second == lenX))
		{
			return true;
		}
	}
	return false;
}

bool CudaEngine_ScoreCalcs::AreBarPeriodPairsScanned(const std::vector<std::pair<BarPeriod, BarPeriod>>& scannedBarPs, BarPeriod barPX, BarPeriod barPY)
{
	for(const auto& pair : scannedBarPs)
	{
		if((pair.first == barPX && pair.second == barPY) || (pair.first == barPY && pair.second == barPX))
		{
			return true;
		}
	}
	return false;
}

ScoresResult CudaEngine_ScoreCalcs::CombineAllResults_ForIdx(int resIdx,dataStream_t* streamResults)
{
	ScoresResult combinedResult;
	for(auto indT : glob_indType_CudaEngine.types)
	{
		IndStream_t streamIndTRes = streamResults->m_indicators[static_cast<int>(indT)];
		for(auto indL : glob_indLen_CudaEngine.lengths)
		{
			LenIndicatorStream_t streamIndT_LenRes = streamIndTRes.m_lens[static_cast<int>(indL)];
			for(auto barP : glob_barP_CudaEngine.periods)
			{
				float* resData = streamIndT_LenRes
					.m_timeFrames[static_cast<int>(barP)]
					.m_valueTypes[static_cast<int>(IndicatorValueType::Value)].m_d_data;
				combinedResult.indicatorScores[static_cast<int>(indT)].m_scores[static_cast<int>(barP)] = resData[resIdx];
			}
		}
	}
}
