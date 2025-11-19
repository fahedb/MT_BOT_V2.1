#include "CudaEngine_ScoreCalcs.h"
#include "Logger.h"
#include "CPP_To_Cuda_Tunnel.h"
#include "StreamSetters.h"
#include "PatternGodUtils.h"
#include "Configrations.h"
#include "TimeLineBuilder.h"
#include <chrono>
#include "ThreadPool.h"
#include <future>


IndicatorTypeList glob_indType_CudaEngine;
IndicatorLenList glob_indLen_CudaEngine;
BarPeriodList glob_barP_CudaEngine;
IndicatorValueTypeList glob_valType_CudaEngine;

#define COMAPRE_WEIGHT 10
#define SCAN_NUMS_TO_FULL_SCAN 100


void CudaEngine_ScoreCalcs::DataStreamSetup(std::vector<ChangeNode*>* tl, dataStream_t** dataSt, DataTimeType tT, bool forceOverWrite)
{
	if (!tl)
	{
		Logger::getInstance().WriteLog("CudaEngine_ScoreCalcs::DataStreamSetup - Invalid data pointer", false, true, LogType::ERR);
		return;
	}
	MTThreadPool* threadPool = MTThreadPool::getInstance();
	std::vector<std::future<void>> futuresPromisses;
	threadPool->Set_PauseLowPriorityTasks(true); // Pause low priority tasks to prioritize this setup

	for (auto indT : glob_indType_CudaEngine.types)
	{
		for (auto indL : glob_indLen_CudaEngine.lengths)
		{
			for (auto barP : glob_barP_CudaEngine.periods)
			{
				for (auto valT : glob_valType_CudaEngine.valueTypes)
				{
					StreamSetters::CheckDataStreamTree(dataSt, indT, indL, barP, valT); // Ensure the data stream tree is properly initialized
					ValueTypeStream_t* valStream = &(*dataSt)->m_indicators[static_cast<int>(indT)].m_lens[static_cast<int>(indL)].m_timeFrames[static_cast<int>(barP)].m_valueTypes[static_cast<int>(valT)];
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
					
					
					auto prom = std::make_shared<std::promise<void>>(); // Create a promise to signal completion
					futuresPromisses.push_back(prom->get_future()); // Store the future for later synchronization

					threadPool->enqueueTask([tl,prom, dataArray, indT, indL, barP, valT]() {
						CudaEngine_ScoreCalcs::InitTimeLineDataIntoArray(tl, dataArray, indT, indL, barP, valT);
						prom->set_value();
						}, TaskPriority::High);

					// Set the data into the appropriate stream
					valStream->m_numElements = tl->size();
					valStream->m_d_data = dataArray;
					
				}
			}
		}
	}
	// Wait for all tasks to complete
	for (auto& f : futuresPromisses)
	{
		f.wait();
	}
	threadPool->Set_PauseLowPriorityTasks(false); // Resume low priority tasks

	// After all data is prepared, set it in the CUDA tunnel
	for(auto indT : glob_indType_CudaEngine.types)
	{
		for(auto indL : glob_indLen_CudaEngine.lengths)
		{
			for(auto barP : glob_barP_CudaEngine.periods)
			{
				for(auto valT : glob_valType_CudaEngine.valueTypes)
				{
					if (tT == DataTimeType::HISTORIC)
					{
						CPP_To_Cuda_Tunnel::Set_HistDataTimeFreameStream((*dataSt)->m_indicators[static_cast<int>(indT)].m_lens[static_cast<int>(indL)].m_timeFrames[static_cast<int>(barP)].m_valueTypes[static_cast<int>(valT)].m_d_data,
							(*dataSt)->m_indicators[static_cast<int>(indT)].m_lens[static_cast<int>(indL)].m_timeFrames[static_cast<int>(barP)].m_valueTypes[static_cast<int>(valT)].m_numElements,
							indT, indL, barP, valT, forceOverWrite);
					}
					else if (tT == DataTimeType::CURRENT)
					{
						CPP_To_Cuda_Tunnel::Set_CurrDataTimeFreameStream((*dataSt)->m_indicators[static_cast<int>(indT)].m_lens[static_cast<int>(indL)].m_timeFrames[static_cast<int>(barP)].m_valueTypes[static_cast<int>(valT)].m_d_data,
							(*dataSt)->m_indicators[static_cast<int>(indT)].m_lens[static_cast<int>(indL)].m_timeFrames[static_cast<int>(barP)].m_valueTypes[static_cast<int>(valT)].m_numElements,
							indT, indL, barP, valT, forceOverWrite);
					}
				}
			}
		}
	}
}

void CudaEngine_ScoreCalcs::DataStreamSetup_Sync(std::vector<ChangeNode*>* tl, dataStream_t** dataSt, DataTimeType tT, bool forceOverWrite)
{

	for (auto indT : glob_indType_CudaEngine.types)
	{
		for (auto indL : glob_indLen_CudaEngine.lengths)
		{
			for (auto barP : glob_barP_CudaEngine.periods)
			{
				for (auto valT : glob_valType_CudaEngine.valueTypes)
				{
					StreamSetters::CheckDataStreamTree(dataSt, indT, indL, barP, valT); // Ensure the data stream tree is properly initialized
					ValueTypeStream_t* valStream = &(*dataSt)->m_indicators[static_cast<int>(indT)].m_lens[static_cast<int>(indL)].m_timeFrames[static_cast<int>(barP)].m_valueTypes[static_cast<int>(valT)];
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

					float* dataArray = new float[tl->size()]; // Allocate array for indicator values

					// Set the data into the appropriate stream
					valStream->m_numElements = tl->size();
					valStream->m_d_data = dataArray;

				}
			}
		}
	}

	for (int i = 0; i < tl->size(); i++)
	{
		ChangeNode* node = tl->at(i);
		if (!node)
			continue;
		DataStreamSetup_Sync_ChangeNodeSet(node, i, dataSt, tT, forceOverWrite);
	}

	// After all data is prepared, set it in the CUDA tunnel
	for (auto indT : glob_indType_CudaEngine.types)
	{
		for (auto indL : glob_indLen_CudaEngine.lengths)
		{
			for (auto barP : glob_barP_CudaEngine.periods)
			{
				for (auto valT : glob_valType_CudaEngine.valueTypes)
				{
					if (tT == DataTimeType::HISTORIC)
					{
						CPP_To_Cuda_Tunnel::Set_HistDataTimeFreameStream((*dataSt)->m_indicators[static_cast<int>(indT)].m_lens[static_cast<int>(indL)].m_timeFrames[static_cast<int>(barP)].m_valueTypes[static_cast<int>(valT)].m_d_data,
							(*dataSt)->m_indicators[static_cast<int>(indT)].m_lens[static_cast<int>(indL)].m_timeFrames[static_cast<int>(barP)].m_valueTypes[static_cast<int>(valT)].m_numElements,
							indT, indL, barP, valT, forceOverWrite);
					}
					else if (tT == DataTimeType::CURRENT)
					{
						CPP_To_Cuda_Tunnel::Set_CurrDataTimeFreameStream((*dataSt)->m_indicators[static_cast<int>(indT)].m_lens[static_cast<int>(indL)].m_timeFrames[static_cast<int>(barP)].m_valueTypes[static_cast<int>(valT)].m_d_data,
							(*dataSt)->m_indicators[static_cast<int>(indT)].m_lens[static_cast<int>(indL)].m_timeFrames[static_cast<int>(barP)].m_valueTypes[static_cast<int>(valT)].m_numElements,
							indT, indL, barP, valT, forceOverWrite);
					}
				}
			}
		}
	}
}

void CudaEngine_ScoreCalcs::DataStreamSetup_Sync_ChangeNodeSet(ChangeNode* node,int idx, dataStream_t** dataSt, DataTimeType tT, bool forceOverWrite)
{
	for (auto indT : glob_indType_CudaEngine.types)
	{
		for (auto indL : glob_indLen_CudaEngine.lengths)
		{
			IndicatorValues* indVals = node->Get_Indicator_Pointer(indT, indL)->Get_IndicatorValues_Pointer();
			if (!indVals)
				continue;
		
			LenIndicatorStream_t* lenStream = &(*dataSt)->m_indicators[static_cast<int>(indT)].m_lens[static_cast<int>(indL)];
			
			for (auto barP : glob_barP_CudaEngine.periods)
			{
				TimeFrameStream_t* timeFrameStream = &lenStream->m_timeFrames[static_cast<int>(barP)];
				for (auto valT : glob_valType_CudaEngine.valueTypes)
				{
					ValueTypeStream_t* valStream = &timeFrameStream->m_valueTypes[static_cast<int>(valT)];
					float* dataArray = valStream->m_d_data;
					if (node)
					{
						if (indVals)
						{
							double value = indVals->Get_Value(valT, barP);
							dataArray[idx] = static_cast<float>(value);
						}
						else
						{
							dataArray[idx] = -1.0f; // Default value if indicator is missing
						}
					}
					else
					{
						dataArray[idx] = -1.0f; // Default value if node is null
					}
				}
			}
		}
	}
}

void CudaEngine_ScoreCalcs::ResultStreamSetup(float** dataSt, int resSize, bool forceOverWrite)
{
	if(!(*dataSt) || forceOverWrite)
	{
		if (*dataSt)
		{
			delete[] *dataSt; // Free existing memory before overwriting
			*dataSt = nullptr;
		}
		*dataSt = new float[resSize]; // Allocate results stream
	}
	memset(*dataSt, 0, sizeof(float) * resSize); // Initialize to zero
	CPP_To_Cuda_Tunnel::Set_TotalScore_D_Memory(resSize); // Reset results stream in CUDA tunnel
}

void CudaEngine_ScoreCalcs::ResetResultStreamsToZero()
{
	CPP_To_Cuda_Tunnel::Reset_TotalScores_D_To0(); // Reset total scores in CUDA tunnel
	int totalScoreSize = (m_historicData->size() - m_currentData->size());
	memset(m_h_totalScores, 0, sizeof(float) * totalScoreSize);

}


void CudaEngine_ScoreCalcs::InitTimeLineDataIntoArray(std::vector<ChangeNode*>* tl, float* dataArray, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType valT)
{
	for (size_t i = 0; i < tl->size(); i++)
	{
		ChangeNode* node = tl->at(i);
		if (node)
		{
			IndicatorValues* indicatorVals = node->Get_Indicator_Pointer(indT, indL)->Get_IndicatorValues_Pointer();
			if (indicatorVals)
			{
				double value = indicatorVals->Get_Value(valT, barP);
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
}

void CudaEngine_ScoreCalcs::Initialize()
{
	if (!m_currentData || !m_historicData)
	{
		Logger::getInstance().WriteLog("CudaEngine_ScoreCalcs::Initialize - Invalid data pointers", false, true, LogType::ERR);
		return;
	}
	ResultStreamSetup(&m_h_totalScores, m_historicData->size(), false); // Setup results stream without overwriting existing data
	DataStreamSetup_Sync(m_historicData, &m_h_histDataStream, DataTimeType::HISTORIC , false); // Setup historic data stream without overwriting existing data
	DataStreamSetup(m_currentData, &m_h_currDataStream,DataTimeType::CURRENT ,true); // Setup current data stream with overwriting existing data
	m_initialized = true;
}


void CudaEngine_ScoreCalcs::RunCudaScan(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D, int scoreToAct)
{
	ResetResultStreamsToZero();

	for (auto indT : glob_indType_CudaEngine.types)
	{
		RunScan_PerIndicator(indT);
	}

	for (auto indT : glob_indType_CudaEngine.types)
	{
		RunScan_TwoIndicatorsRatio(indT);
	}
	CPP_To_Cuda_Tunnel::SyncCUDADevice(); // Wait for all CUDA calculations to complete
	int limit = static_cast<int>(m_historicData->size() - m_currentData->size());
	CPP_To_Cuda_Tunnel::Get_ResultsFromDevice(m_h_totalScores, limit);
	// TODO: Collect results from m_h_totalScores and populate resultNodes2D
	int maxScore = m_h_totalScores[0];
	
	for (size_t i = 0; i < limit; i++)
	{
		if (m_h_totalScores[i] >= scoreToAct)
		{
			if(m_h_totalScores[i] > maxScore)
			{
				maxScore = static_cast<int>(m_h_totalScores[i]);
			}

			ScoresResult scoreResult;//= CombineAllResults_ForIdx(i, m_h_results);
			scoreResult.totalScore.total = static_cast<int>(m_h_totalScores[i]);
			scoreResult.SetLockedResult(true);
			resultNodes2D.push_back(std::make_pair(m_historicData->at(i + m_currentData->size() - 1), scoreResult));
		}
	}
	m_maxScore = maxScore;
	m_scoreToAct = (maxScore - Configrations::getInstance()->Get_Policy<ScoreComparerPolicy>(PolicyType::ScoreComparerPolicy)->Diff_Between_Max_And_ScoreToAct) / m_currentData->size();
	

	if (m_scanCounter % SCAN_NUMS_TO_FULL_SCAN == 0)
	{
		m_useFullScan = true; // Reset to full scan every SCAN_NUMS_TO_FULL_SCAN scans
	}
	else
		m_useFullScan = false;

	m_scanCounter++;
}

CudaEngine_ScoreCalcs::CudaEngine_ScoreCalcs(std::vector<ChangeNode*>* historicData, std::vector<ChangeNode*>* currentData)
{

	m_historicData = historicData;
	m_currentData = currentData;
	m_historyDB_Map = new DBMap(); // Initialize the history DB map
	TimeLineBuilder::Build_TimeLineDB(m_historicData, m_historyDB_Map); // Build the DB map for the history time line
	m_scoreToAct = Configrations::getInstance()->Get_Policy<ScoreComparerPolicy>(PolicyType::ScoreComparerPolicy)->ScoreToAct;
	Initialize();
}

void CudaEngine_ScoreCalcs::Set_NewCurrentTimeLine(std::vector<ChangeNode*>* currentData)
{
	m_currentData = currentData;
	DataStreamSetup(m_currentData, &m_h_currDataStream,DataTimeType::CURRENT, true); // Setup current data stream with overwriting existing data

}

void CudaEngine_ScoreCalcs::Set_NewHistoricTimeLine(std::vector<ChangeNode*>* historicData)
{
	m_historicData = historicData;
	DataStreamSetup(m_historicData, &m_h_histDataStream, DataTimeType::HISTORIC, true); // Setup historic data stream with overwriting existing data
}

bool CudaEngine_ScoreCalcs::IsCudaDeviceAvailable() const
{
	return CPP_To_Cuda_Tunnel::IsCudaAvailable();
}

bool CudaEngine_ScoreCalcs::ScanPreCondition() const
{
	if(!m_currentData || !m_historicData || !m_initialized)
	{
		Logger::getInstance().WriteLog("CudaEngine_ScoreCalcs::ScanPreCondition - Invalid data pointers", false, true, LogType::ERR);
		return false;
	}
	return true;
}
int CudaEngine_ScoreCalcs::ScanForOneMinPatterns_CUDA(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D)
{
	Logger& logger = Logger::getInstance();
	if(!ScanPreCondition())
	{
		logger.WriteLog("CudaEngine_ScoreCalcs::ScanForOneMinPatterns_CUDA - Scan pre-condition failed", false, false, LogType::ERR);
		return 0;
	}
	auto cudaCalcStart = std::chrono::high_resolution_clock::now();
	RunCudaScan(resultNodes2D, m_scoreToAct * m_currentData->size());
	auto cudaCalcEnd = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> cudaCalcDuration = cudaCalcEnd - cudaCalcStart;
	logger.WriteLog("CUDA Score Calculations Time: " + std::to_string(cudaCalcDuration.count()) + " ms", false, false);

	logger.WriteLog("CUDA Scan Results Before Filtering: " + std::to_string(resultNodes2D.size()), false, false);
	
	ScoringLayerPolicy* scoringCompPolicy = Configrations::getInstance()->Get_Policy<ScoringLayerPolicy>(PolicyType::ScoringLayerPolicy);

	auto cudaFilterStart = std::chrono::high_resolution_clock::now();
	PatternGodUtils::FilterDuplicatesTimeLines(resultNodes2D, m_historyDB_Map);
	auto cudaFilterEnd = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> cudaFilterDuration = cudaFilterEnd - cudaFilterStart;
	logger.WriteLog("CUDA Duplicate Filtering Time: " + std::to_string(cudaFilterDuration.count()) + " ms", false, false);

	auto cudaSortStart = std::chrono::high_resolution_clock::now();
	PatternGodUtils::SortScores(resultNodes2D);
	auto cudaSortEnd = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> cudaSortDuration = cudaSortEnd - cudaSortStart;
	logger.WriteLog("CUDA Score Sorting Time: " + std::to_string(cudaSortDuration.count()) + " ms", false, false);

	auto cudaFinalFilterStart = std::chrono::high_resolution_clock::now();
	PatternGodUtils::filterByHighScores(resultNodes2D, scoringCompPolicy->Num_Of_Top_Results_To_Filter_Out);
	auto cudaFinalFilterEnd = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> cudaFinalFilterDuration = cudaFinalFilterEnd - cudaFinalFilterStart;
	logger.WriteLog("CUDA Final Filtering Time: " + std::to_string(cudaFinalFilterDuration.count()) + " ms", false, false);
	logger.WriteLog(PatternGodUtils::BuildScanResultLog(resultNodes2D), false, true);
	//logger.WriteLog(PatternGodUtils::BuildScanResultLog(resultNodes2D), false, true);
	return static_cast<int>(resultNodes2D.size());

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
	float* resData = m_h_totalScores;
	int numResults = m_historicData->size();

	CPP_To_Cuda_Tunnel::Calc_Indicator_ScoreCalcs(resData, numResults, indT, indL, barP, COMAPRE_WEIGHT, m_useFullScan);
}

void CudaEngine_ScoreCalcs::RunScan_TwoIndicatorsRatio(IndicatorType indT)
{
	// Meed to scan for every LenX and LenY --NOTE Scanned(LenX,LenY) == Scanned(LenY,LenX) so we can skip half of the scans
	std::vector<std::pair<IndicatorLen, IndicatorLen>> scannedLens;
	for(auto lenX : glob_indLen_CudaEngine.lengths)
	{
		for(auto lenY : glob_indLen_CudaEngine.lengths)
		{
			if (!AreLenPairsScanned(scannedLens, lenX, lenY))
			{
				RunScan_TwoIndicatorsRatio_IndLens(indT, lenX, lenY);
				scannedLens.push_back(std::make_pair(lenX, lenY));
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
			if (!AreBarPeriodPairsScanned(scannedBarPs, barPX, barPY))
			{
				RunScan_TwoIndicatorsRatio_BarPeriods(indT, lenX, barPX, LenY, barPY);
				scannedBarPs.push_back(std::make_pair(barPX, barPY));
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
	float* resData = m_h_totalScores;
	int numResults = m_historicData->size();

	CPP_To_Cuda_Tunnel::Calc_TwoIndicatorsRatio_ScoreCalcs(resData, numResults, indT, lenX, barPX, LenY, barPY, COMAPRE_WEIGHT / 2, m_useFullScan);
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
	combinedResult.calcTotalResult();
	return combinedResult;
}
