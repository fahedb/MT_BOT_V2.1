#include <vector>
//#include "IndicatorStructres.h"
#include "ChangeData.h"
#include "PatternGod_BaseComparer.h"

#pragma once
class PatternGod_ResultFusionEngine
{
public:
	static std::vector<std::pair<ChangeNode*, ScoresResult>> FuseResults(robin_hood::unordered_map<PatternGodComparerType, std::vector<std::pair<ChangeNode*, ScoresResult>>>* results)
	{
		// STEP 1: FUSE SIMILAR RESULTS
		// STEP 2: NEED TO FIGURE OUT HOW TO CONTINUE FUSING RESULTS

	}


	static std::vector<std::pair<ChangeNode*, ScoresResult>> FuseSimilarResults(robin_hood::unordered_map<PatternGodComparerType, std::vector<std::pair<ChangeNode*, ScoresResult>>>* results)
	{
        struct TempResult {
            int count = 0;
			robin_hood::unordered_map<PatternGodComparerType, ScoresResult> scores; // key = comparer type
        };

		PatternGodComparerTypeList comparerTypeList; // List of comparer types to iterate over
		PatternGodComparerTypeWeights comparerWeights; // Weights for each comparer type

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

        for (std::unordered_map<long long, TempResult>::const_iterator it = appearances.begin(); it != appearances.end(); ++it)
        {
            long long timeKey = it->first;
            const TempResult& entry = it->second;

            if (entry.count >= comparerTypeList.types.size() && nodePointers.count(timeKey))
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
};

