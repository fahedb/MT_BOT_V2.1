#pragma once
#include "KernelExecutionContext.h"
#include "BaseScoringEngine.h"
#include "ChangeData.h"
#include <vector>

#define NUM_OF_SKIPS_FOR_LOW_SCORE_SCAN 10 // Number of skips for low score scan

class PatternGod_ScoreComparer
{
public:
	PatternGod_ScoreComparer(std::vector<ChangeNode*>* history, std::vector<ChangeNode*>* curr, const std::string& scanFilters);

	~PatternGod_ScoreComparer() {}

	void RunScan(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D);
	void Set_NewCurrentTimeLine(std::vector<ChangeNode*>* currentData);
private:
	std::vector<ChangeNode*>* m_historicData;
	std::vector<ChangeNode*>* m_currentData;
	std::vector <BaseScoringEngine*> m_scoringEngines; // Vector to hold different scoring engines for various indicators and combinations

	KernelExecutionContext_PatternGod* m_kernelMemoryContext; // Kernel execution context for managing memory and data streams for CUDA calculations
	void InitComaprer(std::string scanFilters);
	void SetTimeLines(std::vector<ChangeNode*>* history, std::vector<ChangeNode*>* current);
	BaseScoringEngine* CreateScoringEngineForIndicator(const std::string& filter);

};

