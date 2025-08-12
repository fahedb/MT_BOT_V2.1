#include "MinMaxFinder.h"

inline ScoresResult MinMaxFinderScoresResult::Find_Min() const {
    ScoresResult res;
    res.Reset();
    
    if(m_data.empty()) {
        return res; // Return empty result if no data is available
	}

	BarPeriodList barPeriods;
	IndicatorTypeList indTypes;
    for (const auto& indType : indTypes.types)
    {
        IndicatorScore minIndScore;
		minIndScore.Reset(); // Reset the indicator score to ensure it's empty before calculation
        for (const auto& barP : barPeriods.periods)
        {
			minIndScore.scores.at(barP) = Find_MinScore(indType, barP);
        }
		res.indicatorScores[indType] = minIndScore; // Set the minimum scores for the indicator type
    }
    res.calcTotalResult(); // Calculate the total result for the scores
    return res;
}

inline ScoresResult MinMaxFinderScoresResult::Find_Max() const
{
    ScoresResult res;
	res.Reset(); // Reset the result to ensure it's empty before calculation
    
    if (m_data.empty()) {
        return res; // Return empty result if no data is available
    }

	BarPeriodList barPeriods;
	IndicatorTypeList indTypes;

    for (const auto& indType : indTypes.types)
    {
		IndicatorScore maxIndScore;
		maxIndScore.Reset(); // Reset the indicator score to ensure it's empty before calculation
        for (const auto& barP : barPeriods.periods)
        {
			maxIndScore.scores.at(barP) = Find_MaxScore(indType, barP);
		}
		res.indicatorScores[indType] = maxIndScore; // Set the maximum scores for the indicator type
	}
	res.calcTotalResult(); // Calculate the total result for the scores
    return res;
}

double MinMaxFinderScoresResult::Find_MaxScore(IndicatorType indType, BarPeriod barP) const
{
	// Find the maximum score for a specific indicator type and bar period
    double max = std::numeric_limits<double>::lowest();
    for (const auto& score : m_data)
    {
        if (score.indicatorScores.at(indType).scores.at(barP) > max)
        {
            max = score.indicatorScores.at(indType).scores.at(barP);
        }
    }
	return max;
}

double MinMaxFinderScoresResult::Find_MinScore(IndicatorType indType, BarPeriod barP) const
{
    // Find the minimum score for a specific indicator type and bar period
    double min = std::numeric_limits<double>::max();
    for (const auto& score : m_data)
    {
        if (score.indicatorScores.at(indType).scores.at(barP) < min)
        {
            min = score.indicatorScores.at(indType).scores.at(barP);
        }
    }
	return min;
}
