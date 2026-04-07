#include "PatternGod_ScoreComparer.h"
#include <iomanip>
#include <sstream>
#include "Logger.h"
#include <mutex>
#include <future>
#include "Converts.h"
#include "TimeLineArrayBuilder.h"
#include "CPP_To_Cuda_Tunnel.h"

#include "GPUScoringEngine.h"
#include "CPUScoringEngine.h"

std::vector<std::string> SplitString(const std::string& str, char delimiter)
{
	std::vector<std::string> result;
	std::stringstream ss(str);
	std::string item;

	while (std::getline(ss, item, delimiter))
	{
		result.push_back(item);
	}

	return result;
}
void PatternGod_ScoreComparer::Set_NewCurrentTimeLine(std::vector<ChangeNode*>* currentData)
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

PatternGod_ScoreComparer::PatternGod_ScoreComparer(std::vector<ChangeNode*>* history, std::vector<ChangeNode*>* curr, const std::string& scanFilters)
{
	SetTimeLines(history, curr);
	InitComaprer(scanFilters);
	
}

void PatternGod_ScoreComparer::RunScan(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D)
{
	if(m_scoringEngines.empty())
	{
		Logger::getInstance().WriteLog("No scoring engines initialized. RunScan will not execute.", true, true, LogType::ERR);
		return;
	}
	std::vector<int> resultsIndices; // This will hold the indices of the results from the scoring engines. The actual scores can be retrieved from the kernel memory context or directly from the scoring engines depending on your implementation.
	for(const auto& engine : m_scoringEngines)
	{
		
		resultsIndices = engine->RunScan(resultsIndices);
	}
	
	float* totalScores = m_kernelMemoryContext->Get_HostTotalScoresPtr();
	int maxScore = 0;

	for(int index : resultsIndices)
	{
		if(index < 0 || index >= m_historicData->size())
		{
			Logger::getInstance().WriteLog("Invalid index from scoring engine results: " + std::to_string(index), true, true, LogType::ERR);
			continue;
		}
		ChangeNode* node = (*m_historicData)[index];

		int score = totalScores[index];
		ScoresResult scoreResult; // This is a placeholder. You would need to populate this ScoresResult based on the actual scores calculated by the scoring engines for this index.
		scoreResult.totalScore.total = score;
		scoreResult.SetLockedResult(true); // Lock the result to prevent further modifications if needed
		resultNodes2D.emplace_back(node, scoreResult); // Add the node and its corresponding score result to the results vector
		
		if(score > maxScore)
			maxScore = score; // Update max score for potential use in filtering or logging
	}
	Logger::getInstance().WriteLog("Scan completed. Total results: " + std::to_string(resultNodes2D.size()) + ", Max Score: " + std::to_string(maxScore), true, true, LogType::LOG);

}

void PatternGod_ScoreComparer::InitComaprer(std::string scanFilters)
{
	std::string localScanFilters = scanFilters;
	if(scanFilters.empty())
	{
		// F.1 = TIME FRAME, F.2 = THRESHOLD, F.3 = SCAN MODE (SINGLE INDICATOR OR COMBINATION OR ALL)
		localScanFilters = "H4:0.20:S;H4&H1:0.70:D;H1:0.5:S;H1&M30:0.70:D;M30:0.5:S;M30&M15:0.70:D;M15:0.5:S;M15&M5:0.70:D;M5:0.5:S;M5&M1:0.7:D;M1:0.5:S;A:0.6:A";
		Logger::getInstance().WriteLog("ScoreComparerPolicy scanFilters is empty. defulat value initialized: " + localScanFilters, true, true, LogType::LOG);
	}

	bool isCudaAvailable = CPP_To_Cuda_Tunnel::IsCudaAvailable();

	std::vector<std::string> filterParts = SplitString(localScanFilters, ';');
	bool firstFilter = true;
	for(const std::string& part : filterParts)
	{
		BaseScoringEngine* engine = CreateScoringEngineForIndicator(part, !firstFilter);
		firstFilter = false;
		std::cout << "Engine Details, " << "Period Filter size: " << std::to_string(engine->m_barPeriod_Filter.size()) << ", Threshold: " << engine->m_threshold;
		if(engine)
		{
			m_scoringEngines.push_back(engine);
			Logger::getInstance().WriteLog("Initialized " + std::string(isCudaAvailable ? "GPU" : "CPU") + " scoring engine for filter: " + part, true, true, LogType::LOG);
		}
		else
		{
			Logger::getInstance().WriteLog("Failed to initialize scoring engine for filter: " + part, true, true, LogType::ERR);
		}
		
	}

}

void PatternGod_ScoreComparer::SetTimeLines(std::vector<ChangeNode*>* history, std::vector<ChangeNode*>* current)
{
	m_historicData = history;
	m_currentData = current;
	bool isCudaAvailable = CPP_To_Cuda_Tunnel::IsCudaAvailable();

	m_kernelMemoryContext = new KernelExecutionContext_PatternGod(m_historicData->size(), m_currentData->size(), isCudaAvailable);

	dataStream_t* histDataSt = m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::HISTORIC, MemoryLocation::HOST);
	TimeLineArrayBuilder::BuildTimeLineArrays(histDataSt, m_historicData); // Build historic data stream arrays

	dataStream_t* histMinSt = m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::HOST);
	dataStream_t* histMaxSt = m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::HISTORIC, MemoryLocation::HOST);
	int limit = static_cast<int>(m_historicData->size() - m_currentData->size()) + 1;
	TimeLineArrayBuilder::BuildMinMaxArrays(histDataSt, histMinSt, histMaxSt, limit, m_currentData->size());
	if(isCudaAvailable)
		m_kernelMemoryContext->SyncHistoryDataToDevice(true); // Sync historic data to device

	dataStream_t* currDataSt = m_kernelMemoryContext->Get_IndicatorsStreamDataPtr(DataTimeType::CURRENT, MemoryLocation::HOST);
	TimeLineArrayBuilder::BuildTimeLineArrays(currDataSt, m_currentData); // Build current data stream arrays

	dataStream_t* currMinSt = m_kernelMemoryContext->Get_MinValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::HOST);
	dataStream_t* currMaxSt = m_kernelMemoryContext->Get_MaxValuesStreamPtr(DataTimeType::CURRENT, MemoryLocation::HOST);
	limit = 1; // For current data, limit is always 1
	TimeLineArrayBuilder::BuildMinMaxArrays(currDataSt, currMinSt, currMaxSt, limit, m_currentData->size());
	if(isCudaAvailable)
		m_kernelMemoryContext->SyncCurrentDataToDevice(true); // Sync current data to device

}
// H4:0.20:S = H4 period, 0.20 threshold, Single indicator scan mode
BaseScoringEngine* PatternGod_ScoreComparer::CreateScoringEngineForIndicator(const std::string& filter, bool foceCPUEngine)
{
	std::vector<std::string> keyValue = SplitString(filter, ':');
	bool isCudaAvailable = CPP_To_Cuda_Tunnel::IsCudaAvailable();

	if (keyValue.size() != 3)
	{
		Logger::getInstance().WriteLog("Invalid scan filter format: " + filter, true, true, LogType::ERR);
		return nullptr;
	}
	std::string indPeriodsStr = keyValue[0];
	float threshold = std::stof(keyValue[1]);
	if (threshold <= 0 || threshold > 1)
	{
		Logger::getInstance().WriteLog("Invalid threshold in scan filter: " + filter, true, true, LogType::ERR);
		return nullptr;
	}
	
	// For simplicity, we will create a generic CudaScoringEngine that can handle different indicators and lengths based on the filter. In a real implementation, you might want to have specific scoring engine classes for different indicators or combinations.
	IndicatorLenList lenList; // Currently includes all lengths, but can be extended to filter specific lengths based on the filter string in the future
	IndicatorTypeList indTypeList; // Currently filters only by bar period, but this can be extended to include indicator types and lengths in the future

	std::vector<BarPeriod> barPeriodFilter;
	
	std::vector<std::string> periodsStrs = SplitString(indPeriodsStr, '&');
	for(const std::string& perStr : periodsStrs)
	{
		if(perStr == "A") // If the period is "A", it means all periods, so we add all possible periods to the filter and break the loop
		{
			BarPeriodList allPeriods; // This will include all possible bar periods
			barPeriodFilter = allPeriods.periods;
			break;
		}
		BarPeriod bp = BarPeriodUtils::StringToBarPeriod(perStr);
		if(bp == BarPeriod::NONE)
		{
			Logger::getInstance().WriteLog("Invalid bar period in scan filter: " + filter, true, true, LogType::ERR);
			return nullptr;
		}
		barPeriodFilter.push_back(bp);
	}

	if (barPeriodFilter.empty())
	{
		Logger::getInstance().WriteLog("No valid bar periods found in scan filter: " + filter, true, true, LogType::ERR);
		return nullptr;
	}

	BaseScoringEngine* engine = nullptr;
	if(isCudaAvailable && !foceCPUEngine)
	{

		engine = new GPUScoringEngine(lenList.lengths, barPeriodFilter, indTypeList.types, threshold);
	}
	else
	{
		if (foceCPUEngine)
			Logger::getInstance().WriteLog("Forced CPU Engine creation for filter: " + filter, true, true, LogType::LOG);
		else
			Logger::getInstance().WriteLog("CUDA is not available. CPU Engine going to be created for filter: " + filter, true, true, LogType::LOG);

		engine = new CPUScoringEngine(lenList.lengths, barPeriodFilter, indTypeList.types, threshold);
	}
	engine->Set_KernelMemoryContext(m_kernelMemoryContext); // Set the kernel memory context for the scoring engine
	std::cout << "Engine Details, " << "Period Filter size: " << std::to_string(engine->m_barPeriod_Filter.size()) << ", Threshold: " << engine->m_threshold;
	std::string scanMode = keyValue[2];
	if (scanMode == "S") // Single indicator scan
	{
		engine->Enable_SingleIndicatorScan(true);
		engine->Enable_TwoIndicatorsRatioScan(false);
	}
	else if (scanMode == "D") // Dual indicators ratio scan
	{
		engine->Enable_SingleIndicatorScan(false);
		engine->Enable_TwoIndicatorsRatioScan(true);
	}
	else if (scanMode == "A") // All scan modes
	{
		engine->Enable_SingleIndicatorScan(true);
		engine->Enable_TwoIndicatorsRatioScan(true);
	}
	else
	{
		Logger::getInstance().WriteLog("Invalid scan mode in scan filter: " + filter, true, true, LogType::ERR);
		delete engine; // Clean up the engine if it was created
		return nullptr;
	}
	return engine;
}


