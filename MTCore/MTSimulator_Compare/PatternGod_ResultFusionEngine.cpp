#include "PatternGod_ResultFusionEngine.h"
#include "TimeLineBuilder.h"
#include "Configrations.h"
#include "Normalizer.h"
#include "MinMaxFinder.h"
#include "LOGGER.H"

std::vector<std::pair<ChangeNode*, ScoresResult>> PatternGod_ResultFusionEngine::FuseSimilarResults(robin_hood::unordered_map<PatternGodComparerType, std::vector<std::pair<ChangeNode*, ScoresResult>>>* results)
{
    struct TempResult {
        int count = 0;
        robin_hood::unordered_map<PatternGodComparerType, ScoresResult> scores; // key = comparer type
    };

    PatternGodComparerTypeList comparerTypeList; // List of comparer types to iterate over
    PatternGodComparerTypeWeights comparerWeights(*Configrations::getInstance()->Get_Policy<BaseComparerPolicy>(PolicyType::BaseComparerPolicy)); // Weights for each comparer type

    std::unordered_map<long long, TempResult> appearances;           // key = point_DateTimeLong
    std::unordered_map<long long, ChangeNode*> nodePointers;         // to retrieve ChangeNode* by time

    // --------------------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------------------

    // Step1 : Collecting appearances and scores for each ChangeNode
    for (typename robin_hood::unordered_map<PatternGodComparerType, std::vector<std::pair<ChangeNode*, ScoresResult>>>::const_iterator it = results->begin();
        it != results->end(); ++it)
    {
        PatternGodComparerType comparerType = it->first;
        const std::vector<std::pair<ChangeNode*, ScoresResult>>& resultVec = it->second;

        for (size_t i = 0; i < resultVec.size(); ++i)
        {
            ChangeNode* node = resultVec[i].first;
            const ScoresResult& score = resultVec[i].second;
            long long timeKey = node->point_DateTimeLong;

            TempResult& entry = appearances[timeKey];
            entry.count++;

            entry.scores[comparerType] = score; // Store the score for each comparer type

            nodePointers[timeKey] = node;
        }
    }

    // --------------------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------------------

    // Step2 : Fusing results based on the count of appearances
    std::vector<std::pair<ChangeNode*, ScoresResult>> fusedResults;
	int minSimResultsToFuse = Configrations::getInstance()->Get_Policy<BaseComparerPolicy>(PolicyType::BaseComparerPolicy)->Min_Similar_Results_To_Fusion;
    for (std::unordered_map<long long, TempResult>::const_iterator it = appearances.begin(); it != appearances.end(); ++it)
    {
        long long timeKey = it->first;
        const TempResult& entry = it->second;

        if (entry.count >= minSimResultsToFuse && nodePointers.count(timeKey))
        {
            // Calc the final score by applying weights on ScoresResult objects
            ScoresResult finalScore;

            for (const auto& scoreEntry : entry.scores)
            {
                PatternGodComparerType comparerType = scoreEntry.first;
                const ScoresResult& score = scoreEntry.second;
                // Apply the weight for the current comparer type

                double weight = 0;
                auto itWeight = comparerWeights.weights.find(comparerType);
                if (itWeight == comparerWeights.weights.end())
                {
                    Logger::getInstance().WriteLog("Weight for comparer type " + std::to_string(static_cast<int>(comparerType)) + " not found !!! \nYou will have Wrong results in calcs !!!!!!", false, true, LogType::ERR);
                    weight = 1.0; // Default weight if not found
                }
                else {
                    weight = itWeight->second;
                }

                finalScore += score * weight; // Weighted total score
            }

            finalScore.calcTotalResult(); // Recalculate the total score after applying weights
            fusedResults.emplace_back(nodePointers[timeKey], finalScore);
        }
    }

    return fusedResults;
}

inline PatternGodComparerResultsMap PatternGod_ResultFusionEngine::Get_NormalizedComparerMap(PatternGodComparerResultsMap& results)
{
    PatternGodComparerResultsMap normalizedResults;
    PatternGodComparerTypeList comparerTypes;

    for (const auto& comparerType : comparerTypes.types)
    {
        auto scores = TimeLineBuilder::SplitScoresFromChangeNode(&results.at(comparerType));  // Split the scores from the change nodes
        auto changeNodes = TimeLineBuilder::SplitChangeNodeFromScores(&results.at(comparerType)); // Split the change nodes from the scores

        MinMaxFinderScoresResult minMaxFinder(scores); // Create a MinMaxFinderScoresResult object to find the min and max scores
        ScoresResultRangeNormalizer normalizer(minMaxFinder.Find_Min(), minMaxFinder.Find_Max()); // Create a normalizer using the min and max scores

        for (int i = 0; i < scores.size(); i++) // Replace Results with normalized once
        {
            scores.at(i) = normalizer.normalize(scores.at(i), NORMALIZE_WEIGHT); // Normalize the scores using the normalizer	
        }

        auto normalizedResPairs = TimeLineBuilder::CombineScoresAndChangeNodes(changeNodes, scores); // Combine the normalized scores with the change nodes

        normalizedResults[comparerType] = normalizedResPairs;
    }
    return normalizedResults;
}
