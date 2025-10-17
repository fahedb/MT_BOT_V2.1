#pragma once
#include "ChangeData.h"
#include "MTStreamsStructs.h"
#include "Configrations.h"

class CudaEngine_ScoreCalcs
{
private:
	std::vector<ChangeNode*>* m_historicData = nullptr;
	std::vector<ChangeNode*>* m_currentData = nullptr;
	
	dataStream_t* m_h_results = nullptr;
	float* m_h_totalScores = nullptr;
	dataStream_t* m_h_histDataStream = nullptr;
	dataStream_t* m_h_currDataStream = nullptr;

	int m_scoreToAct = 0;
public:
	CudaEngine_ScoreCalcs()
	{
	}
	CudaEngine_ScoreCalcs(std::vector<ChangeNode*>* historicData, std::vector<ChangeNode*>* currentData)
	{
		m_historicData = historicData;
		m_currentData = currentData;
		m_scoreToAct = Configrations::getInstance()->Get_Policy<ScoreComparerPolicy>(PolicyType::ScoreComparerPolicy)->ScoreToAct;
		Initialize();
	}
	void RunCudaScan(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D, int scoreToAct, int numThreads);
	void Set_NewCurrentTimeLine(std::vector<ChangeNode*>* currentData);
	void Set_NewHistoricTimeLine(std::vector<ChangeNode*>* historicData);
private:
	void RunScan_PerIndicator(IndicatorType indT);
	void RunScan_PerIndicator_IndLen(IndicatorType indT, IndicatorLen indL);
	void RunScan_PerIndicator_BarPeriod(IndicatorType indT, IndicatorLen indL, BarPeriod barP);
	
	void RunScan_TwoIndicatorsRatio(IndicatorType indT);
	void RunScan_TwoIndicatorsRatio_IndLens(IndicatorType indT, IndicatorLen lenX, IndicatorLen LenY);
	void RunScan_TwoIndicatorsRatio_BarPeriods(IndicatorType indT, IndicatorLen lenX, BarPeriod barPX, IndicatorLen LenY, BarPeriod barPY);
	static bool AreLenPairsScanned(const std::vector<std::pair<IndicatorLen, IndicatorLen>>& scannedLens, IndicatorLen lenX, IndicatorLen lenY);
	static bool AreBarPeriodPairsScanned(const std::vector<std::pair<BarPeriod, BarPeriod>>& scannedBarPs, BarPeriod barPX, BarPeriod barPY);

	static ScoresResult CombineAllResults_ForIdx(int resIdx, dataStream_t* streamResults);

	void Initialize();
	void DataStreamSetup(std::vector<ChangeNode*>* tl, dataStream_t* dataSt, bool forceOverWrite = false);
	void ResultStreamSetup(dataStream_t* dataSt, int resSize, bool forceOverWrite = false);
	void ResetStreamToZero(dataStream_t* dataSt);

};

