#pragma once
#include <string>
#include <vector>
#include <cmath>
#include "TimeLine.h"
#include "TimeLineScored.h"
#include "CoinsPair.h"

enum class CommonDirection
{
	UP = 0,
	DOWN = 1,
	BLUR = 3,
	NONE = 4 // Added NONE to handle cases where direction is not defined
};

class CommonDirection_Utils
{
public:
	static std::string GetCommonDirectionStr(CommonDirection dir);
};

enum class StatsType
{
	General = 0,
	First = 1,
	Highest =2
};

struct StepsMean
{
	std::vector<int> stepsSum;
	std::vector<int> stepsCount;

	void addStep(int pipChange, int candleIdx)
	{
		if (stepsSum.size() <= candleIdx) {
			stepsSum.resize(static_cast<size_t>(candleIdx) + static_cast<size_t>(1), 0); // Resize the vector with candleIdx + 1, and init it with 0
			stepsCount.resize(static_cast<size_t>(candleIdx) + static_cast<size_t>(1), 0);
		}
		stepsSum[candleIdx] += pipChange;
		stepsCount[candleIdx] += 1;
	}
	double GetMeanForCandleIdx(int candleIdx) const {

		if (candleIdx >= stepsSum.size() || stepsCount[candleIdx] == 0)
			return 0.0;
		return static_cast<double>(stepsSum[candleIdx]) / stepsCount[candleIdx];
	}

	std::vector<double> GetAllMeansUpTo(int maxIndex = -1) const {
		std::vector<double> result;
		if (maxIndex < 0 || maxIndex >= stepsSum.size())
			maxIndex = stepsSum.size() - 1;

		result.reserve(static_cast<size_t>(maxIndex + 1));
		
		for (int i = 0; i <= maxIndex; ++i)
			result.push_back(GetMeanForCandleIdx(i));
		return result;
	}
};

struct DirectionGroupStats
{
	int Total = 0;
	int UpCount = 0;
	int DownCount = 0;
	int BlurCount = 0;
	double SumPips = 0.0;
	double SumSquaredPips = 0.0;
	std::vector <std::pair<int, int>> Indexes;

	void add(double pipChange, CommonDirection dir, int candleIdx) {
		Total++;
		SumPips += pipChange;
		SumSquaredPips += pipChange * pipChange;
		Indexes.push_back(std::make_pair(candleIdx, pipChange));

		switch (dir) {
		case CommonDirection::UP: UpCount++; break;
		case CommonDirection::DOWN: DownCount++; break;
		case CommonDirection::BLUR: BlurCount++; break;
		}
	}

	double mean() const {
		return (Total == 0) ? 0.0 : SumPips / Total;
	}

	double pure_mean(CommonDirection dir) {
		int total = 0;
		int sum = 0;
		for (int i = 0; i < Indexes.size(); i++)
		{
			if (pureDirection(Indexes.at(i).second) == dir)
			{
				total++;
				sum += Indexes.at(i).second;
			}
		}
		return (total == 0) ? 0.0 : sum / total;
	}

	double meanIndexes() const {
		int indexesSum = 0;

		for (int i = 0; i < Indexes.size(); i++)
		{
			indexesSum += Indexes.at(i).first;
		}
		return double(indexesSum) / double(Indexes.size());
	}
	std::vector < std::pair<int, int>> get_Indexes_and_changePips() { return Indexes; }

	double stddev() const {
		if (Total == 0) return 0.0;
		double avg = mean();
		return std::sqrt((SumSquaredPips / Total) - (avg * avg));
	}
private:
	CommonDirection pureDirection(int pipChange)
	{
		if (pipChange > 0) return CommonDirection::UP;
		if (pipChange < 0) return CommonDirection::DOWN;
		return CommonDirection::BLUR;
	}

};


class DirectionStats {
private:
	DirectionGroupStats general;
	DirectionGroupStats first;
	DirectionGroupStats highest;
	StepsMean stepsMean_General;
	StepsMean stepsMean_PurePositive;
	StepsMean stepsMean_PureNegative;
	std::vector<TimeLineScored> m_patterns_wScores;
	TimeLine* m_currPattern;
	bool m_tradeRes;
	bool m_active;
	std::string m_timePoint;
	CoinsPair m_pair;


	DirectionGroupStats& GetGroupStats(StatsType type);

public:
	DirectionStats(DirectionStats&& other) noexcept
		: m_patterns_wScores(std::move(other.m_patterns_wScores)),
		m_currPattern(other.m_currPattern ? new TimeLine(std::move(*other.m_currPattern)) : nullptr),
		m_tradeRes(other.m_tradeRes),
		m_active(other.m_active),
		m_timePoint(std::move(other.m_timePoint)),
		m_pair(other.m_pair),
		general(std::move(other.general)),
		first(std::move(other.first)),
		highest(std::move(other.highest)),
		stepsMean_General(std::move(other.stepsMean_General)), // Move constructor
		stepsMean_PurePositive(std::move(other.stepsMean_PurePositive)), // Move constructor
		stepsMean_PureNegative(std::move(other.stepsMean_PureNegative)) // Move constructor
	{
		other.m_currPattern = nullptr; // Nullify the moved pointer
	}

	DirectionStats(const DirectionStats& other)
		: m_patterns_wScores(other.m_patterns_wScores),
		m_currPattern(other.m_currPattern ? new TimeLine(*other.m_currPattern) : nullptr ),
		m_tradeRes(other.m_tradeRes),
		m_active(other.m_active),
		m_timePoint(other.m_timePoint),
		m_pair(other.m_pair),
		general(other.general),
		first(other.first),
		highest(other.highest),
		stepsMean_General(other.stepsMean_General),  // Copy constructor
		stepsMean_PurePositive(other.stepsMean_PurePositive), // Copy constructor
		stepsMean_PureNegative(other.stepsMean_PureNegative)  // Copy constructor
	{
	}

	DirectionStats()
	{
		m_tradeRes = false;
		m_currPattern = nullptr;
		m_pair = CoinsPair::NONE;
		m_active = false;
	}

	~DirectionStats()
	{
		if (m_currPattern)
		{
			delete m_currPattern;
			m_currPattern = nullptr;
		}
	}

	DirectionStats& operator=(const DirectionStats&) = delete; // Delete copy assignment operator 

	void Activate() { m_active = true; }
	bool IsActive() { return m_active; }
	void Set_TradeRes(bool tradeRes) { m_tradeRes = tradeRes; }
	bool Get_TradeRes() { return m_tradeRes; };

	std::vector<double> GetAllMeansUpTo(CommonDirection dir, int maxIndex = -1) { return GetStepMeans(dir).GetAllMeansUpTo(maxIndex); }// Ensure the means are calculated for the specified direction

	
	void add(int pipChange, CommonDirection dir, int candleIdx, StatsType sType) { GetGroupStats(sType).add(pipChange, dir, candleIdx); }
	
	void addStep_ForCandleIdx(int pipChange, int candleIdx) {
		stepsMean_General.addStep(pipChange, candleIdx);

		if (pipChange > 0) {
			stepsMean_PurePositive.addStep(pipChange, candleIdx);
		}
		else if (pipChange < 0) {
			stepsMean_PureNegative.addStep(pipChange, candleIdx);

		}
	}

	int Get_UpCount(StatsType sType) { return GetGroupStats(sType).UpCount; }
	int Get_DownCount(StatsType sType) { return GetGroupStats(sType).DownCount; }
	int Get_BlurCount(StatsType sType) { return GetGroupStats(sType).BlurCount; }
	int Get_TotalCount(StatsType sType) { return GetGroupStats(sType).Total; }

	TimeLine* Get_CurrPattern() { return m_currPattern; }
	std::vector<TimeLineScored>* Get_SimilarPatterns() { return &m_patterns_wScores; }

	double Get_PureMean(StatsType sType, CommonDirection dir) { return GetGroupStats(sType).pure_mean(dir); }

	double stddev(StatsType sType) { return GetGroupStats(sType).stddev(); }
	double mean(StatsType sType) { return GetGroupStats(sType).mean(); }
	double meanIndexes(StatsType sType) { return GetGroupStats(sType).meanIndexes(); }
	
	double Get_Mean_For_CandleIdx(int candleIdx) { return stepsMean_General.GetMeanForCandleIdx(candleIdx); }
	std::vector < std::pair<int, int>> Get_Indexes_and_changePips(StatsType sType) { return GetGroupStats(sType).get_Indexes_and_changePips(); }

	void Add_Pattern(TimeLineScored pattern) { m_patterns_wScores.push_back(pattern); }
	void Add_Patterns(std::vector<TimeLineScored>* patterns,bool moveMemory = false);
	void Add_CurrPattern(TimeLine&& currPatter) 
	{ 
		m_currPattern = new TimeLine(std::move(currPatter));
	}
	void Add_CurrPattern(TimeLine& currPatter,bool moveMemory = false) 
	{
		if (moveMemory)
			m_currPattern = new TimeLine(std::move(currPatter));
		else
			m_currPattern = new TimeLine(currPatter);
	}

	void Set_PairCoint(CoinsPair pair) { m_pair = pair; }
	CoinsPair Get_PairCoint(CoinsPair& pair) { return m_pair; }

	void Set_TimePoint(const std::string& timePoint) { m_timePoint = timePoint; }
	std::string Get_TimePoint() { return m_timePoint; }

	DirectionGroupStats& GetGroupStatsByName(const std::string& son);
	StepsMean& GetStepMeans(CommonDirection dir) 
	{
		if (dir == CommonDirection::UP) return stepsMean_PurePositive;
		if (dir == CommonDirection::DOWN) return stepsMean_PureNegative;
		return stepsMean_General; // Default to General if not UP or DOWN
	}
};
