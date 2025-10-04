#include "VotingStruct.h"
#include <cmath>

robin_hood::unordered_map<CommonDirection, double> VotedDirectionStats::GetVotedPercentages() // Return percentages for each direction based on weights
{
	// Calculate percentages by weights
	double upSum = 0.0;
	double downSum = 0.0;
	double blurSum = 0.0;
	double totalSum = 0.0;
	for (double w : UpCounts) upSum += w;
	for (double w : DownCounts) downSum += w;
	for (double w : BlurCounts) blurSum += w;
	totalSum = upSum + downSum + blurSum;
	robin_hood::unordered_map<CommonDirection, double> percentages;
	if (totalSum > 0.0)
	{
		percentages[CommonDirection::UP] = (upSum / totalSum) * 100.0;
		percentages[CommonDirection::DOWN] = (downSum / totalSum) * 100.0;
		percentages[CommonDirection::BLUR] = (blurSum / totalSum) * 100.0;
	}
	else
	{
		percentages[CommonDirection::UP] = 0.0;
		percentages[CommonDirection::DOWN] = 0.0;
		percentages[CommonDirection::BLUR] = 0.0;
	}
	return percentages;

}

double VotedDirectionStats::pure_mean(CommonDirection dir) const {
	double total = Total;
	double sum = 0.0;
	switch (dir)
	{
	case CommonDirection::UP:
		sum = UpPipsChange_Total;
		break;
	case CommonDirection::DOWN:
		sum = DownPipsChange_Total;
		break;
	case CommonDirection::BLUR:
		sum = 0;
		break;
	case CommonDirection::NONE:
		sum = 0;
		break;
	default:
		break;
	}

	return (total == 0) ? 0.0 : sum / total;
}

double VotedDirectionStats::stddev() const {
	if (Total == 0) return 0.0;
	double avg = mean();
	return std::sqrt((SumSquaredPips / Total) - (avg * avg));
}

double VotedDirectionStats::meanIndexes() const {
	int indexesSum = 0;

	for (int i = 0; i < pipsChangeTillIndex.size(); i++)
	{
		indexesSum += pipsChangeTillIndex.at(i).first;
	}
	return double(indexesSum) / double(pipsChangeTillIndex.size());
}

void VotedDirectionStats::Add_Sums(CommonDirection dir, double weight, int pipsChange, int candleIdx)
{
	Total++;
	PipsChange_Total += pipsChange;
	SumSquaredPips += pipsChange * pipsChange;
	if (dir == CommonDirection::UP) {
		UpPipsChange_Total += pipsChange;
		UpSumSquaredPips += pipsChange * pipsChange;
	}
	else if (dir == CommonDirection::DOWN)
	{
		DownPipsChange_Total += pipsChange;
		DownSumSquaredPips += pipsChange * pipsChange;
	}
	pipsChangeTillIndex.push_back(std::make_pair(candleIdx, pipsChange)); // Store the pips change till that index

}

void VotedDirectionStats::Add_Weights(CommonDirection dir, double weight)
{
	switch (dir)
	{
	case CommonDirection::UP:
		UpCounts.push_back(weight);
		break;
	case CommonDirection::DOWN:
		DownCounts.push_back(weight);
		break;
	case CommonDirection::BLUR:
		BlurCounts.push_back(weight);
		break;
	case CommonDirection::NONE:
		break;
	default:
		break;
	}
}
