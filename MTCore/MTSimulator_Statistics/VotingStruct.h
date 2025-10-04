#pragma once

#include <vector>
#include "..\MyRobinHoodMap\robin_hood.h"
#include "CommonDirection.h"

class VotedDirectionStats
{
	int Total = 0; // Total number of votes
	std::vector<double> UpCounts; // Store Weight for each vote -- index 0 = first vote, index 1 = second vote ...
	std::vector<double> DownCounts; // Store Weight for each vote -- index 0 = first vote, index 1 = second vote ...
	std::vector<double> BlurCounts; // Store Weight for each vote -- index 0 = first vote, index 1 = second vote ...

	int UpPipsChange_Total = 0; // Total pips change for UP votes
	int DownPipsChange_Total = 0; // Total pips change for DOWN votes
	int PipsChange_Total = 0; // Total pips change for all votes

	double UpSumSquaredPips = 0.0; // Sum of squared pips changes for UP votes
	double DownSumSquaredPips = 0.0; // Sum of squared pips changes for DOWN votes
	double SumSquaredPips = 0.0; // Sum of squared pips changes for all votes

	std::vector <std::pair<int, int>> pipsChangeTillIndex; // Store pair of <index, pipsChange> for each vote -- every cell should contain the future candle index per pattern and the pips change till that index
public:
	void add(CommonDirection dir, double weight, int pipsChange, int candleIdx) {
		Add_Weights(dir, weight);
		Add_Sums(dir, weight, pipsChange, candleIdx);
	}
	
	robin_hood::unordered_map<CommonDirection, double> GetVotedPercentages(); // Get the voted percentages for each direction based on weights
	int getTotal() const { return Total; } // Get total number of votes
	double mean() const { return (Total == 0) ? 0.0 : static_cast<double>(PipsChange_Total) / Total; } // Mean pips change for all votes
	double pure_mean(CommonDirection dir) const; // Mean pips change for a specific direction
	double stddev() const; // Standard deviation of pips changes for all votes
	double meanIndexes() const; // Mean of the indexes where pips changes occurred
	std::vector<std::pair<int, int>> get_Indexes_and_changePips() { return pipsChangeTillIndex; } // Get the vector of pairs containing indexes and corresponding pips changes
private:

	void Add_Sums(CommonDirection dir, double weight, int pipsChange, int candleIdx); // Add sums and counts based on direction and pips change
	void Add_Weights(CommonDirection dir, double weight); // Add weights based on direction

};

