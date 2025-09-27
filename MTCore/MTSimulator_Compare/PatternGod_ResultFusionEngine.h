#include <vector>
//#include "IndicatorStructres.h"
#pragma once
#include "PatternGod_BaseComparer.h"

#define NORMALIZE_WEIGHT 100 // Normalization weight for scores

class PatternGod_ResultFusionEngine
{
public:
	static std::vector<std::pair<ChangeNode*, ScoresResult>> FuseResults(PatternGodComparerResultsMap& results, DBMap* historyDB_Map)
	{
		// Get Normalized results
		//PatternGodComparerResultsMap normalizedResults = Get_NormalizedComparerMap(results);


		// STEP 1: FUSE SIMILAR RESULTS
		std::vector<std::pair<ChangeNode*, ScoresResult>> similarResultsFused = FuseSimilarResults(&results); // Call the function to fuse similar results based on comparer types
		

		// STEP 4: Filter duplicates and return the final results
		PatternGodUtils::FilterDuplicatesTimeLines(similarResultsFused, historyDB_Map);
		return similarResultsFused; // Return the final fused results


	}

	static std::vector<std::pair<ChangeNode*, ScoresResult>> FuseSimilarResults(robin_hood::unordered_map<PatternGodComparerType, std::vector<std::pair<ChangeNode*, ScoresResult>>>* results);

private:
	static PatternGodComparerResultsMap Get_NormalizedComparerMap(PatternGodComparerResultsMap& results);

};

