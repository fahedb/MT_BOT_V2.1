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

	}
};

