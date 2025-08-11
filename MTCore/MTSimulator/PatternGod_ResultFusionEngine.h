#include <vector>
//#include "IndicatorStructres.h"
#include "ChangeData.h"
#include "PatternGod_BaseComparer.h"

#pragma once
class PatternGod_ResultFusionEngine
{
public:
	static std::vector<std::pair<ChangeNode*, ScoresResult>> FuseResults(robin_hood::unordered_map<PatternGodComparerType, std::vector<std::pair<ChangeNode*, ScoresResult>>>* results, DBMap* historyDB_Map)
	{
		// STEP 1: FUSE SIMILAR RESULTS
        auto similarResultsFused = FuseSimilarResults(results); // Call the function to fuse similar results based on comparer types
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
};

