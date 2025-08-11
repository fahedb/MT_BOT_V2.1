#include <vector>
//#include "IndicatorStructres.h"
#pragma once
#include "PatternGod_BaseComparer.h"
#include "Normalizer.h"

class PatternGod_ResultFusionEngine
{
public:
	static std::vector<std::pair<ChangeNode*, ScoresResult>> FuseResults(PatternGodComparerResultsMap& results, DBMap* historyDB_Map)
	{
		// Normalize all results
		
		//TimeLineBuilder::SplitScoresFromChangeNode(results); // Split ChangeNode from ScoresResult for normalization
		// STEP 1: FUSE SIMILAR RESULTS
        auto similarResultsFused = FuseSimilarResults(&results); // Call the function to fuse similar results based on comparer types
		// STEP 2: Fuse and get highest scores using weights
		//auto weightFusedResults = FuseResultsByWeights(results); // Fuse results by weights

		// STEP 3: Combine all results
		std::vector<std::pair<ChangeNode*, ScoresResult>> allResults(similarResultsFused.begin(), similarResultsFused.end());
		//allResults.insert(allResults.end(), weightFusedResults.begin(), weightFusedResults.end());

		// STEP 4: Filter duplicates and return the final results
		PatternGodUtils::FilterDuplicatesTimeLines(allResults, historyDB_Map);
		return allResults; // Return the final fused results

	}
	static std::vector<std::pair<ChangeNode*, ScoresResult>> FuseResultsByWeights(robin_hood::unordered_map<PatternGodComparerType, std::vector<std::pair<ChangeNode*, ScoresResult>>>* results)
	{
		
	}

	static std::vector<std::pair<ChangeNode*, ScoresResult>> FuseSimilarResults(robin_hood::unordered_map<PatternGodComparerType, std::vector<std::pair<ChangeNode*, ScoresResult>>>* results);

private:
	static PatternGodComparerResultsMap Get_NormalizedComparerMap(PatternGodComparerResultsMap& results)
	{
		PatternGodComparerResultsMap normalizedMap;
		for (const auto& pair : results)
		{
			auto tempScoreVec = TimeLineBuilder::SplitScoresFromChangeNode(&pair.second);
			auto tempChNVec = TimeLineBuilder::SplitChangeNodeFromScores(&pair.second);
			// Need to nomalize
		}
	}

};

