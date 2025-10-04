#pragma once
#include <vector>
#include "ChangeData.h"
class BaseVote
{
public:
	virtual std::vector<double> CalcVoteWeights(std::vector<std::pair<ChangeNode*, ScoresResult>>* potentioalRes);
private:
	virtual std::vector<double> CalculateWeights(const std::vector<double>& normalizedScores);
	virtual double CalculateWeight(double normalizedScore) = 0; // Pure virtual function to calculate weight based on method and normalized score
};

