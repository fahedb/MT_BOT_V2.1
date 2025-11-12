#include "VotingStruct.h"
#include <cmath>

void VotedDirectionStats::ImportStatsData(double upPercent, double downPercent, double blurPercent,
	double meanIndexes, double meanPipsChange,
	double pureMeanPipsChange_UP, double pureMeanPipsChange_DOWN,
	double stdDevPipsChange, int totalVotes)
{
	Imported_UpPercentage = upPercent;
	Imported_DownPercentage = downPercent;
	Imported_BlurPercentage = blurPercent;
	Imported_Mean_Indexes = meanIndexes;
	Imported_Mean_PipsChange = meanPipsChange;
	Imported_PureMeanPipsChange_UP = pureMeanPipsChange_UP;
	Imported_PureMeanPipsChange_DOWN = pureMeanPipsChange_DOWN;
	Imported_StdDevPipsChange = stdDevPipsChange;
	Imported_TotalVotes = totalVotes;
	importedData_Filled_Flag = true;
}
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
	
	if(importedData_Filled_Flag)
	{
		percentages[CommonDirection::UP] = Imported_UpPercentage;
		percentages[CommonDirection::DOWN] = Imported_DownPercentage;
		percentages[CommonDirection::BLUR] = Imported_BlurPercentage;
		return percentages;
	}

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
	
	if(importedData_Filled_Flag)
	{
		if (dir == CommonDirection::UP)
			return Imported_PureMeanPipsChange_UP;
		else if (dir == CommonDirection::DOWN)
			return Imported_PureMeanPipsChange_DOWN;
		else
			return 0.0;
	}

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
	if(importedData_Filled_Flag)
	{
		return Imported_StdDevPipsChange;
	}
	if (Total == 0) return 0.0;
	double avg = mean();
	return std::sqrt((SumSquaredPips / Total) - (avg * avg));
}

double VotedDirectionStats::meanIndexes() const {
	if(importedData_Filled_Flag)
	{
		return Imported_Mean_Indexes;
	}

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

double VotedDirectionStats::mean() const // Mean pips change for all votes
{
	if (importedData_Filled_Flag)
	{
		return Imported_Mean_PipsChange;
	}
	return (Total == 0) ? 0.0 : static_cast<double>(PipsChange_Total) / Total;
}