#pragma once
#include <string>
#include <vector>
#include <cmath>

enum class CommonDirection
{
	UP = 0,
	DOWN = 1,
	BLUR = 3
};
enum class StatsType
{
	General = 0,
	First = 1
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

		result.reserve(static_cast<size_t>(maxIndex) + static_cast<size_t>(1));
		
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

};

struct DirectionStats {
	std::string timePoint;
	DirectionGroupStats general;
	DirectionGroupStats first;
	StepsMean stepsMean;

	DirectionGroupStats& GetGroupStats(StatsType type)
	{
		switch (type)
		{
		case StatsType::General:
			return general;
			break;
		case StatsType::First:
			return first;
			break;
		default:
			return general;
			break;
		}
	}
	DirectionGroupStats& GetGroupStatsByName(const std::string& son)
	{
		if (son == "First") return first;
		if (son == "General") return general;
		return general;
	}
	void addStep_ForCandleIdx(double pipChange, int candleIdx) { stepsMean.addStep(pipChange, candleIdx); }
	void add(double pipChange, CommonDirection dir, int candleIdx, StatsType sType)
	{
		if (sType == StatsType::First) first.add(pipChange, dir, candleIdx);
		if (sType == StatsType::General) general.add(pipChange, dir, candleIdx);
	}
	double mean(StatsType sType) { return GetGroupStats(sType).mean(); }
	double meanIndexes(StatsType sType) { return GetGroupStats(sType).meanIndexes(); }
	double stddev(StatsType sType) { return GetGroupStats(sType).stddev(); }
	std::vector < std::pair<int, int>> get_Indexes_and_changePips(StatsType sType) { return GetGroupStats(sType).get_Indexes_and_changePips(); }

	int Get_UpCount(StatsType sType) { return GetGroupStats(sType).UpCount; }
	int Get_DownCount(StatsType sType) { return GetGroupStats(sType).DownCount; }
	int Get_BlurCount(StatsType sType) { return GetGroupStats(sType).BlurCount; }
	int Get_TotalCount(StatsType sType) { return GetGroupStats(sType).Total; }
	double Get_Mean_For_CandleIdx(int candleIdx) { return stepsMean.GetMeanForCandleIdx(candleIdx); }
	std::vector<double> GetAllMeansUpTo(int maxIndex = -1) { return stepsMean.GetAllMeansUpTo(maxIndex); }
};

