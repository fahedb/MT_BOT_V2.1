#pragma once
#include "ChangeData.h"
#include "MTStreamsStructs.h"
#include "DBMap.h"
#include "KernelExecutionContext.h"


class CudaEngine_ScoreCalcs
{
private:
	std::vector<ChangeNode*>* m_historicData = nullptr;
	std::vector<ChangeNode*>* m_currentData = nullptr;
	
	KernelExecutionContext_PatternGod* m_kernelMemoryContext = nullptr;

	DBMap* m_historyDB_Map = nullptr;
	int m_scoreToAct = 0;
	int m_maxScore = 0;
	bool m_useFullScan = true;
	int m_scanCounter = 0;
public:
	CudaEngine_ScoreCalcs(std::vector<ChangeNode*>* historicData, std::vector<ChangeNode*>* currentData);
	void Set_NewCurrentTimeLine(std::vector<ChangeNode*>* currentData);
	void Set_NewHistoricTimeLine(std::vector<ChangeNode*>* historicData);
	bool IsCudaDeviceAvailable() const;
	int ScanForOneMinPatterns_CUDA(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D);
private:
	bool m_initialized = false;
	bool ScanPreCondition() const;
	void RunCudaScan(std::vector<std::pair<ChangeNode*, ScoresResult>>& resultNodes2D, int scoreToAct);

	// --------------------------------------------------------------------------------------------------------------
	////////////////////////////////////////////////////////////
	////////// Single Indicator Scan Routines //////////////////
	////////////////////////////////////////////////////////////
	void RunScan_PerIndicator(IndicatorType indT);
	void RunScan_PerIndicator_IndLen(IndicatorType indT, IndicatorLen indL);
	void RunScan_PerIndicator_BarPeriod(IndicatorType indT, IndicatorLen indL, BarPeriod barP);
	// --------------------------------------------------------------------------------------------------------------
	////////////////////////////////////////////////////////////////
	////////// Two Indicators Ratio Scan Routines //////////////////
	///////////////////////////////////////////////////////////////
	void RunScan_TwoIndicatorsRatio(IndicatorType indT);
	void RunScan_TwoIndicatorsRatio_IndLens(IndicatorType indT, IndicatorLen lenX, IndicatorLen LenY);
	void RunScan_TwoIndicatorsRatio_BarPeriods(IndicatorType indT, IndicatorLen lenX, BarPeriod barPX, IndicatorLen LenY, BarPeriod barPY);
	// --------------------------------------------------------------------------------------------------------------
	///////////////////////////////////////////////////////////
	////////// Static Helper Functions ////////////////////////
	///////////////////////////////////////////////////////////
	static int CalcTotalResultsForIdx(int resIdx, dataStream_t* streamResults);
	static bool AreBarPeriodPairsScanned(const std::vector<std::pair<BarPeriod, BarPeriod>>& scannedBarPs, BarPeriod barPX, BarPeriod barPY);
	static bool AreLenPairsScanned(const std::vector<std::pair<IndicatorLen, IndicatorLen>>& scannedLens, IndicatorLen lenX, IndicatorLen lenY);
	static float* Get_DataPointer(dataStream_t* dataSt,IndicatorType indT, IndicatorLen indL, BarPeriod barP, IndicatorValueType val);
	// --------------------------------------------------------------------------------------------------------------
	/////////////////////////////////////////////////////////
	////////// Stream Initialization //////////////
	/////////////////////////////////////////////////////////
	void Initialize();
	
};

