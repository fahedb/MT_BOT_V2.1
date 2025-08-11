#include "MinMaxFinder.h"

inline ScoresResult MinMaxFinderScoresResult::Find_Min() const {
    double min = std::numeric_limits<double>::max();
    ScoresResult res;
    for (const auto& score : m_data)
    {
        if (score.totalScore.total < min)
        {
            min = score.totalScore.total;
            res = score;
        }
    }
    return res;
}

inline ScoresResult MinMaxFinderScoresResult::Find_Max() const
{
    double max = std::numeric_limits<double>::lowest();
    ScoresResult res;
    for (const auto& score : m_data)
    {
        if (score.totalScore.total > max)
        {
            res = score;
            max = score.totalScore.total;
        }
    }
    return res;
}
