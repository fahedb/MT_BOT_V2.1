#include <vector>
//#include "IndicatorStructres.h"
#pragma once
#include "PatternGod_BaseComparer.h"
#include "Normalizer.h"
#include "MinMaxFinder.h"
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
	static PatternGodComparerResultsMap Get_NormalizedComparerMap(PatternGodComparerResultsMap& results)
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

};

