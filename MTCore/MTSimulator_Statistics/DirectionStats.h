#pragma once
#include <string>
#include <vector>
#include <cmath>
#include "TimeLine.h"
#include "TimeLineScored.h"
#include "CoinsPair.h"
#include "..\MyRobinHoodMap\robin_hood.h"
#include "CommonDirection.h"
#include "StatsTypeEnum.h"
#include "VotingStruct.h"
#include "StepsMean.h"


class DirectionStats {
private:
	VotedDirectionStats general;
	VotedDirectionStats first;
	VotedDirectionStats highest;

	StepsMean stepsMean_General;
	StepsMean stepsMean_PurePositive;
	StepsMean stepsMean_PureNegative;

	std::vector<TimeLineScored> m_patterns_wScores;
	TimeLine* m_currPattern;
	bool m_tradeRes;
	bool m_active;
	std::string m_timePoint;
	CoinsPair m_pair;


	VotedDirectionStats& GetGroupStats(StatsType type);

public:
	DirectionStats(DirectionStats&& other) noexcept;

	DirectionStats(const DirectionStats& other);

	DirectionStats();

	~DirectionStats()
	{
		if (m_currPattern)
		{
			delete m_currPattern;
			m_currPattern = nullptr;
		}
	}

	DirectionStats& operator=(const DirectionStats&) = delete; // Delete copy assignment operator 

	void								Activate() { m_active = true; }
	bool								IsActive() { return m_active; }

	void								add(CommonDirection dir, double weight, int pipChange, int candleIdx, StatsType sType) { GetGroupStats(sType).add(dir, weight, pipChange, candleIdx); }
	void								addStep_ForCandleIdx(int pipChange, int candleIdx);

	int Get_Total(StatsType sType) { return GetGroupStats(sType).getTotal(); }
	robin_hood::unordered_map<CommonDirection, double> Get_VotedPercentages(StatsType sType) { return GetGroupStats(sType).GetVotedPercentages(); }

	TimeLine*							Get_CurrPattern() { return m_currPattern; }
	VotedDirectionStats&				GetGroupStatsByName(const std::string& son);
	StepsMean&							GetStepMeans(CommonDirection dir);
	double								Get_Mean_For_CandleIdx(int candleIdx) { return stepsMean_General.GetMeanForCandleIdx(candleIdx); }
	double								Get_PureMean(StatsType sType, CommonDirection dir) { return GetGroupStats(sType).pure_mean(dir); }
	bool								Get_TradeRes() { return m_tradeRes; };
	std::vector<double>					GetAllMeansUpTo(CommonDirection dir, int maxIndex = -1) { return GetStepMeans(dir).GetAllMeansUpTo(maxIndex); }// Ensure the means are calculated for the specified direction
	std::vector<TimeLineScored>*		Get_SimilarPatterns() { return &m_patterns_wScores; }
	std::vector < std::pair<int, int>>	Get_Indexes_and_changePips(StatsType sType) { return GetGroupStats(sType).get_Indexes_and_changePips(); }
	std::string							Get_TimePoint() { return m_timePoint; }
	CoinsPair							Get_PairCoint(CoinsPair& pair) { return m_pair; }
	
	double								stddev(StatsType sType) { return GetGroupStats(sType).stddev(); }
	double								mean(StatsType sType) { return GetGroupStats(sType).mean(); }
	double								meanIndexes(StatsType sType) { return GetGroupStats(sType).meanIndexes(); }
	
	void								Add_Pattern(TimeLineScored pattern) { m_patterns_wScores.push_back(pattern); }
	void								Add_Patterns(std::vector<TimeLineScored>* patterns,bool moveMemory = false);
	void								Add_CurrPattern(const TimeLine& currPatter,bool moveMemory = false);

	void								Set_PairCoint(CoinsPair pair) { m_pair = pair; }
	void								Set_TimePoint(const std::string& timePoint) { m_timePoint = timePoint; }
	void								Set_TradeRes(bool tradeRes) { m_tradeRes = tradeRes; }
};

