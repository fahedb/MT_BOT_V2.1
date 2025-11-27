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
#include "TimeLineArrayBuilder.h"
#include "GPU_LaunchesParameters.h"


IndicatorTypeList glob_indType_CudaEngine;
IndicatorLenList glob_indLen_CudaEngine;
BarPeriodList glob_barP_CudaEngine;
IndicatorValueTypeList glob_valType_CudaEngine;

#define COMAPRE_WEIGHT 10
#define SCAN_NUMS_TO_FULL_SCAN 100


void CudaEngine_ScoreCalcs::Initialize()
{
	if (!m_currentData || !m_historicData)
	{
		Logger::getInstance().WriteLog("CudaEngine_ScoreCalcs::Initialize - Invalid data pointers", false, true, LogType::ERR);
		return;
	}
	m_kernelMemoryContext = new KernelExecutionContext_PatternGod(m_historicData->size(), m_currentData->size());
	Set_NewHistoricTimeLine(m_historicData); // Set historic data to device
	Set_NewCurrentTimeLine(m_currentData); // Set current data to device

	m_initialized = true;
}


void CudaEngine_ScoreCalcs::RunCudaScan(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D, int scoreToAct)
{
	m_kernelMemoryContext->PrepareForNewScan(); // Prepare the kernel context for a new scan
	
	for (auto indT : glob_indType_CudaEngine.types)
	{
		RunScan_PerIndicator(indT);
	}
	CPP_To_Cuda_Tunnel::SyncCUDADevice(); // Wait for all CUDA calculations to complete

	m_kernelMemoryContext->SyncResultsToHost(); // Sync results back to host
	int limit = static_cast<int>(m_historicData->size() - m_currentData->size());
	float* totalScores = m_kernelMemoryContext->Get_HostTotalScoresPtr();
	
	TimeLineArrayBuilder::BuildTotalScoreArray(m_kernelMemoryContext->Get_ResultsDataStreamPtr(MemoryLocation::HOST), totalScores, limit);
	
	std::vector<int> topKScoresIndices = Get_TopKScoresIndices(
		m_kernelMemoryContext->Get_HostTotalScoresPtr(),
		limit,
		(GPU_THREADS_NUM * GPU_BLOCKS_AFTER_FILTER) /* 8 blocks */);

	m_kernelMemoryContext->Import_AfterFilterIndices(topKScoresIndices); // Import top K indices to device for two indicators ratio scan


	for (auto indT : glob_indType_CudaEngine.types)
	{
		RunScan_TwoIndicatorsRatio(indT);
	}
	CPP_To_Cuda_Tunnel::SyncCUDADevice(); // Wait for all CUDA calculations to complete
	
	m_kernelMemoryContext->SyncResultsToHost(); // Sync results back to host
	TimeLineArrayBuilder::BuildTotalScoreArray(m_kernelMemoryContext->Get_ResultsDataStreamPtr(MemoryLocation::HOST), totalScores, limit);

	std::vector<int> top100ScoresIndices = Get_TopKScoresIndices(
		m_kernelMemoryContext->Get_HostTotalScoresPtr(),
		limit,
		100);

	int maxScore = totalScores[0];
	for(int idx : top100ScoresIndices)
	{
		if(totalScores[idx] > maxScore)
		{
			maxScore = static_cast<int>(totalScores[idx]);
		}
		ScoresResult scoreResult;//= CombineAllResults_ForIdx(i, m_h_results);
		scoreResult.totalScore.total = static_cast<int>(totalScores[idx]);
		scoreResult.SetLockedResult(true);
		resultNodes2D.push_back(std::make_pair(m_historicData->at(idx + m_currentData->size() - 1), scoreResult));
	}

	m_maxScore = maxScore;
	m_scoreToAct = (maxScore - Configrations::getInstance()->Get_Policy<ScoreComparerPolicy>(PolicyType::ScoreComparerPolicy)->Diff_Between_Max_And_ScoreToAct) / m_currentData->size();
	
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
	dataStream_t* currDataSt = m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT, MemoryLocation::HOST);
	TimeLineArrayBuilder::BuildTimeLineArrays(currDataSt, m_currentData); // Build current data stream arrays

	dataStream_t* currMinSt = m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::HOST);
	dataStream_t* currMaxSt = m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::HOST);
	int limit = 1; // For current data, limit is always 1
	TimeLineArrayBuilder::BuildMinMaxArrays(currDataSt, currMinSt, currMaxSt, limit, m_currentData->size());

	m_kernelMemoryContext->SyncCurrentDataToDevice(true); // Sync current data to device

}

void CudaEngine_ScoreCalcs::Set_NewHistoricTimeLine(std::vector<ChangeNode*>* historicData)
{
	m_historicData = historicData;
	dataStream_t* histDataSt = m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC, MemoryLocation::HOST);
	TimeLineArrayBuilder::BuildTimeLineArrays(histDataSt, m_historicData); // Build historic data stream arrays

	dataStream_t* histMinSt = m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::HOST);
	dataStream_t* histMaxSt = m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::HOST);
	int limit = static_cast<int>(m_historicData->size() - m_currentData->size()) + 1;
	TimeLineArrayBuilder::BuildMinMaxArrays(histDataSt, histMinSt, histMaxSt, limit, m_currentData->size());
	m_kernelMemoryContext->SyncHistoryDataToDevice(true); // Sync historic data to device
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
	float* d_resData = Get_DataPointer(m_kernelMemoryContext->Get_ResultsDataStreamPtr(MemoryLocation::DEVICE), indT, indL, barP, IndicatorValueType::Value);
	
	const float* d_histValues = Get_DataPointer(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, indL, barP, IndicatorValueType::Value);
	const float* d_histSlopes = Get_DataPointer(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, indL, barP, IndicatorValueType::Slope);

	const float* d_histMinValues = Get_DataPointer(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, indL, barP, IndicatorValueType::Value);
	const float* d_histMaxValues = Get_DataPointer(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, indL, barP, IndicatorValueType::Value);

	const float* d_histMinSlopes = Get_DataPointer(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, indL, barP, IndicatorValueType::Slope);
	const float* d_histMaxSlopes = Get_DataPointer(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, indL, barP, IndicatorValueType::Slope);

	const float* d_currValues = Get_DataPointer(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, indL, barP, IndicatorValueType::Value);
	const float* d_currSlopes = Get_DataPointer(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, indL, barP, IndicatorValueType::Slope);

	const float* d_currMinValues = Get_DataPointer(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, indL, barP, IndicatorValueType::Value);
	const float* d_currMaxValues = Get_DataPointer(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, indL, barP, IndicatorValueType::Value);

	const float* d_currMinSlopes = Get_DataPointer(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, indL, barP, IndicatorValueType::Slope);
	const float* d_currMaxSlopes = Get_DataPointer(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, indL, barP, IndicatorValueType::Slope);


	int numResults = m_historicData->size();
	int currTimeLineLen = m_currentData->size();
	CPP_To_Cuda_Tunnel::Calc_Indicator_ScoreCalcs(
		d_resData,
		d_histValues, d_histSlopes,
		d_histMinValues, d_histMaxValues,
		d_histMinSlopes, d_histMaxSlopes,

		d_currValues, d_currSlopes,
		d_currMinValues, d_currMaxValues,
		d_currMinSlopes, d_currMaxSlopes,
		currTimeLineLen, numResults, indT, COMAPRE_WEIGHT);
	m_singleKernelsLaunched++;

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
	float* d_resData = Get_DataPointer(m_kernelMemoryContext->Get_ResultsDataStreamPtr(MemoryLocation::DEVICE), indT, lenX, barPX, IndicatorValueType::Value);
	
	////////////// Get IndX data //////////////////	
	const float* d_histIndX_Values = Get_DataPointer(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC,MemoryLocation::DEVICE), indT, lenX, barPX, IndicatorValueType::Value);
	const float* d_histIndX_Slopes = Get_DataPointer(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC,MemoryLocation::DEVICE), indT, lenX, barPX, IndicatorValueType::Slope);

	const float* d_histIndX_MinValues = Get_DataPointer(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, lenX, barPX, IndicatorValueType::Value);
	const float* d_histIndX_MaxValues = Get_DataPointer(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, lenX, barPX, IndicatorValueType::Value);
	const float* d_histIndX_MinSlopes = Get_DataPointer(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, lenX, barPX, IndicatorValueType::Slope);
	const float* d_histIndX_MaxSlopes = Get_DataPointer(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, lenX, barPX, IndicatorValueType::Slope);
	////////////////////////////////////////////////

	////////////// Get curr IndX data //////////////////
	const float* d_currIndX_Values = Get_DataPointer(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT,MemoryLocation::DEVICE), indT, lenX, barPX, IndicatorValueType::Value);
	const float* d_currIndX_Slopes = Get_DataPointer(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT,MemoryLocation::DEVICE), indT, lenX, barPX, IndicatorValueType::Slope);

	const float* d_currIndX_MinValues = Get_DataPointer(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, lenX, barPX, IndicatorValueType::Value);
	const float* d_currIndX_MaxValues = Get_DataPointer(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, lenX, barPX, IndicatorValueType::Value);
	const float* d_currIndX_MinSlopes = Get_DataPointer(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, lenX, barPX, IndicatorValueType::Slope);
	const float* d_currIndX_MaxSlopes = Get_DataPointer(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, lenX, barPX, IndicatorValueType::Slope);
	////////////////////////////////////////////////

	////////////// Get hist IndY data //////////////////
	const float* d_histIndY_Values = Get_DataPointer(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, LenY, barPY, IndicatorValueType::Value);
	const float* d_histIndY_Slopes = Get_DataPointer(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, LenY, barPY, IndicatorValueType::Slope);

	const float* d_histIndY_MinValues = Get_DataPointer(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, LenY, barPY, IndicatorValueType::Value);
	const float* d_histIndY_MaxValues = Get_DataPointer(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, LenY, barPY, IndicatorValueType::Value);
	const float* d_histIndY_MinSlopes = Get_DataPointer(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, LenY, barPY, IndicatorValueType::Slope);
	const float* d_histIndY_MaxSlopes = Get_DataPointer(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::DEVICE), indT, LenY, barPY, IndicatorValueType::Slope);
	/////////////////////////////////////////////////

	////////////// Get curr IndY data //////////////////
	const float* d_currIndY_Values = Get_DataPointer(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, LenY, barPY, IndicatorValueType::Value);
	const float* d_currIndY_Slopes = Get_DataPointer(m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, LenY, barPY, IndicatorValueType::Slope);
	
	const float* d_currIndY_MinValues = Get_DataPointer(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, LenY, barPY, IndicatorValueType::Value);
	const float* d_currIndY_MaxValues = Get_DataPointer(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, LenY, barPY, IndicatorValueType::Value);
	const float* d_currIndY_MinSlopes = Get_DataPointer(m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, LenY, barPY, IndicatorValueType::Slope);
	const float* d_currIndY_MaxSlopes = Get_DataPointer(m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::DEVICE), indT, LenY, barPY, IndicatorValueType::Slope);
	/////////////////////////////////////////////////

	const int* d_afterFilterIndices = m_kernelMemoryContext->Get_AfterFilterIndicesPtr(MemoryLocation::DEVICE);
	const int afterFilterCount = m_kernelMemoryContext->Get_AfterFilterIndicesCount();

	int numResults = m_historicData->size();
	int currTimeLineLen = m_currentData->size();

	CPP_To_Cuda_Tunnel::Calc_CandidateTwoIndicatorRatio_ScoreCalcs(
		d_afterFilterIndices,
		afterFilterCount,
		d_resData,
		///////////////////// Hist len X arrays ////////////////////////////////////////////
		d_histIndX_Values, d_histIndX_Slopes,
		d_histIndX_MinValues, d_histIndX_MaxValues,
		d_histIndX_MinSlopes, d_histIndX_MaxSlopes,
		////////////////////////////////////////////////////////////////////////////////////
		/////////////////////// Current len X arrays //////////////////////////////////////
		d_currIndX_Values, d_currIndX_Slopes,
		d_currIndX_MinValues, d_currIndX_MaxValues,
		d_currIndX_MinSlopes, d_currIndX_MaxSlopes,
		///////////////////////////////////////////////////////////////////////////////////
		////////////////////// Hist len Y arrays //////////////////////////////////////////
		d_histIndY_Values, d_histIndY_Slopes,
		d_histIndY_MinValues, d_histIndY_MaxValues,
		d_histIndY_MinSlopes, d_histIndY_MaxSlopes,
		///////////////////////////////////////////////////////////////////////////////////
		///////////////////// Current len Y arrays ////////////////////////////////////////
		d_currIndY_Values, d_currIndY_Slopes,
		d_currIndY_MinValues, d_currIndY_MaxValues,
		d_currIndY_MinSlopes, d_currIndY_MaxSlopes,
		currTimeLineLen, numResults, COMAPRE_WEIGHT / 2);
	m_twoIndicatorKernelsLaunched++;
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

float* CudaEngine_ScoreCalcs::Get_DataPointer(dataStream_t* dataSt, IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType val)
{
	return dataSt->m_indicators.at(static_cast<int>(indT))
		.m_lens.at(static_cast<int>(indL))
		.m_timeFrames.at(static_cast<int>(barP))
		.m_valueTypes.at(static_cast<int>(val)).m_d_data;
}

std::vector<int> CudaEngine_ScoreCalcs::Get_TopKScoresIndices(float* totalScores, int numOfResults, int topKNum)
{
	// Stage 1: Build indices
	std::vector<int> idx(numOfResults);
	for (int i = 0; i < numOfResults; i++)
		idx[i] = i;

	// Stage 2: nth_element – O(N)
	std::nth_element(
		idx.begin(), idx.begin() + topKNum, idx.end(),
		[&](int a, int b) { return totalScores[a] > totalScores[b]; }
	);

	// Stage 3: sort only the top K – O(K log K)
	std::sort(
		idx.begin(), idx.begin() + topKNum,
		[&](int a, int b) { return totalScores[a] > totalScores[b]; }
	);

	// shrink to topK
	idx.resize(topKNum);
	return idx;
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

int CudaEngine_ScoreCalcs::CalcTotalResultsForIdx(int resIdx,dataStream_t* streamResults)
{
	int totalScore = 0;
	for(auto indT : glob_indType_CudaEngine.types)
	{
		IndStream_t& streamIndTRes = streamResults->m_indicators.at(static_cast<int>(indT));
		for(auto indL : glob_indLen_CudaEngine.lengths)
		{
			LenIndicatorStream_t& streamIndT_LenRes = streamIndTRes.m_lens.at(static_cast<int>(indL));
			for(auto barP : glob_barP_CudaEngine.periods)
			{
				totalScore += streamIndT_LenRes.m_timeFrames.at(static_cast<int>(barP))
					.m_valueTypes.at(static_cast<int>(IndicatorValueType::Value)).m_d_data[resIdx];
			}
		}
	}
	return totalScore;
}
