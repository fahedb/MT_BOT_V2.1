#include "PolynomScoreCalcs.h"
#include "PolynomTimeLineCache.h"
#include "ScoreResultsCachePolynom.h"

const IndicatorTypeList m_glob_indTypes;
const IndicatorLenList m_glob_indLengths;
const IndicatorValueTypeList m_glob_indValueTypes;
const BarPeriodList m_glob_barPeriods;

double CalcSinglePolynomScore(const Polynom* hist, const Polynom* curr, PolynomComparerPolicy* policy, const int weight = 10) {
	if (!hist || !curr)
		return 0.0;

	double wEval = policy->weight_eval;
	double wRMSE = policy->weight_rmse;
	double wCoeff = policy->weight_coeff;


	double scoreEval = PolynomComparer::Compare_ByEval(*hist, *curr);
	double scoreRMSE = PolynomComparer::Compare_ByRMSE(*hist, *curr);
	double scoreCoeff = PolynomComparer::Compare_ByWeightedCoefficients(*hist, *curr);
	double score = wEval * scoreEval + wRMSE * scoreRMSE + wCoeff * scoreCoeff;
	return score * weight;

}

ScoresResult PolynomScoreCalcs::CompareTimeLines(std::vector<ChangeNode*>* hist, std::vector<ChangeNode*>* curr,int threadIdx, int& resCacheHits, int& resCacheMisses, int& polyCacheHits, int& polyCacheMisses)
{
	if (hist->empty() || curr->empty())
		return ScoresResult(); // Return an empty ScoresResult if either vector is empty
	if (hist->size() != curr->size())
	{
		Logger::getInstance().WriteLog("Vectors are not of the same size: " + std::to_string(hist->size()) + " vs " + std::to_string(curr->size()), false, true, LogType::ERR);
		return ScoresResult(); // Return an empty ScoresResult if the vectors are not of the same size
	}
	
	const int polyLen = Configrations::getInstance()->Get_Policy<PolynomComparerPolicy>(PolicyType::PolynomComparerPolicy)->PolynomLen_ForScan;
	ScoreResultsCachePolynom* scoreResultsCache = ScoreResultsCachePolynom::getInstance();
	ScoreCachingPolicy* scoreCachePolicy = Configrations::getInstance()->Get_Policy<ScoreCachingPolicy>(PolicyType::ScoreCachingPolicy);

	ScoresResult totalScore;
	int totalCacheHits = 0;
	int totalCacheMisses = 0;

	int totalPolyCacheHits = 0;
	int totalPolyCacheMisses = 0;

	// Temporary caches for polynomial timelines
	robin_hood::unordered_map<PolynomTimeLineKey, PolynomTimeLine> histPolyCache;
	robin_hood::unordered_map<PolynomTimeLineKey, PolynomTimeLine> currPolyCache;

	for (int i = 0; i < hist->size(); i++)
	{
		if (i + polyLen >= hist->size() || i + polyLen >= curr->size())
			break; // Break if we exceed the size of either vector

		ScoresResult* foundRes = nullptr; // Initialize foundRes to nullptr
		if (scoreCachePolicy->EnableScoreCaching)
			 foundRes = TryFindCachedScore(PolynomTimeLineKey(hist->at(i)->point_DateTimeLong, hist->at(i+polyLen - 1)->point_DateTimeLong ),
										   PolynomTimeLineKey(curr->at(i)->point_DateTimeLong, curr->at(i + polyLen - 1)->point_DateTimeLong)
											, threadIdx); // Try to find the score in the cache

		if (foundRes) // If the score was found in the cache
		{
			totalCacheHits++;
			totalScore += *foundRes; // Add the cached score to the total score
			continue; // Skip to the next iteration if the score was found in the cache
		}
		else
			totalCacheMisses++;

		std::vector<ChangeNode*> partialVec1(
			hist->begin() + (static_cast<size_t>(i)),
			hist->begin() + (static_cast<size_t>(i) + static_cast<size_t>(polyLen)));

		std::vector<ChangeNode*> partialVec2(
			curr->begin() + (static_cast<size_t>(i)),
			curr->begin() + (static_cast<size_t>(i) + static_cast<size_t>(polyLen)));

		
		PolynomTimeLine polyTomeLine1(partialVec1, CacheType::HISTORICAL); // Ensure the polynomials are built and cached
		PolynomTimeLine polyTomeLine2(partialVec2, CacheType::CURRENT); // Ensure the polynomials are built and cached

		if(polyTomeLine2.Get_PolynomPackage()->m_inCache)
			totalPolyCacheHits++;
		else
			totalPolyCacheMisses++;

		TimeLinePolynomScore localScore = CalcScore(polyTomeLine1.Get_PolynomPackage(), polyTomeLine2.Get_PolynomPackage()); // calculate the score for the current polynomials

		totalScore += *localScore.GetScoreContainer(IndicatorValueType::Value);
		totalScore += *localScore.GetScoreContainer(IndicatorValueType::Slope);

		if (!foundRes)
			TryStoreScoreInCache(partialVec1, partialVec2, localScore, threadIdx); // Try to store the score in the cache
		
		// Cache the polynomials if they were not from cache
		if(!polyTomeLine1.Get_PolynomPackage()->m_inCache)
			histPolyCache.try_emplace(PolynomTimeLineKey(partialVec1.front()->point_DateTimeLong, partialVec1.back()->point_DateTimeLong),std::move(polyTomeLine1));
		if (!polyTomeLine2.Get_PolynomPackage()->m_inCache)
			currPolyCache.try_emplace(PolynomTimeLineKey(partialVec2.front()->point_DateTimeLong, partialVec2.back()->point_DateTimeLong), std::move(polyTomeLine2));
	}
	resCacheHits += totalCacheHits;
	resCacheMisses += totalCacheMisses;
	polyCacheHits += totalPolyCacheHits;	
	polyCacheMisses += totalPolyCacheMisses;

	// Merge the newly built polynomial timelines into the singleton cache
	PolynomTimeLineCache::getInstance()->MergeIntoCache(histPolyCache, CacheType::HISTORICAL);
	PolynomTimeLineCache::getInstance()->MergeIntoCache(currPolyCache, CacheType::CURRENT);

	return totalScore; // Return the total score calculated from all polynomial comparisons
}

TimeLinePolynomScore PolynomScoreCalcs::CalcScore(PolynomChangeNodePackage* hist, PolynomChangeNodePackage* curr)
{
	TimeLinePolynomScore score;
	score.GetScoreContainer(IndicatorValueType::Value)->Reset(); // Reset the score to ensure it's empty before calculation
	score.GetScoreContainer(IndicatorValueType::Slope)->Reset(); // Reset the score to ensure it's empty before calculation

	if (hist == nullptr || curr == nullptr) {
		return score; // Return empty score if either package is null
	}
	// Calculate EMA scores for each polynomial

	PolynomComparerPolicy* policy = Configrations::getInstance()->Get_Policy<PolynomComparerPolicy>(PolicyType::PolynomComparerPolicy);

	for(const auto& indType : m_glob_indTypes.types)
	{
		for(const auto& indLen : m_glob_indLengths.lengths)
		{
			for(const auto& valType : m_glob_indValueTypes.valueTypes)
			{
				auto polynomKey = PolynomKey(indType, indLen, valType);
				score.GetScoreContainer(valType)->indicatorScores.at(static_cast<int>(indType)).m_scores[static_cast<int>(BarPeriod::M1)] += CalcSinglePolynomScore(hist->GetPolynom(polynomKey), curr->GetPolynom(polynomKey), policy);
			}
		}
	}

	// Calculate the total score
	score.GetScoreContainer(IndicatorValueType::Value)->calcTotalResult();
	score.GetScoreContainer(IndicatorValueType::Slope)->calcTotalResult();
	return score;
}

ScoresResult* PolynomScoreCalcs::TryFindCachedScore(const PolynomTimeLineKey& histKey, const PolynomTimeLineKey& currKey, int threadIdx)
{
	return ScoreResultsCachePolynom::getInstance()->GetScoreResult(histKey, currKey, threadIdx);
}


bool PolynomScoreCalcs::TryStoreScoreInCache(
	const std::vector<ChangeNode*>& histSegment,
	const std::vector<ChangeNode*>& currSegment,
	TimeLinePolynomScore& score,
	int threadIdx)
{
	if (histSegment.empty() || currSegment.empty())
		return false;

	ScoreCachingPolicy* scoreCachePolicy = Configrations::getInstance()->Get_Policy<ScoreCachingPolicy>(PolicyType::ScoreCachingPolicy);
	if (!scoreCachePolicy->EnableScoreCaching)
		return false;

	ScoreResultsCachePolynom* cache = ScoreResultsCachePolynom::getInstance();

	PolynomTimeLineKey histKey(histSegment.front()->point_DateTimeLong, histSegment.back()->point_DateTimeLong);
	PolynomTimeLineKey currKey(currSegment.front()->point_DateTimeLong, currSegment.back()->point_DateTimeLong);

	ScoresResult scoreResult;
	scoreResult.Reset();
	scoreResult += *(score.GetScoreContainer(IndicatorValueType::Value));
	scoreResult += *(score.GetScoreContainer(IndicatorValueType::Slope));

	cache->StoreNewScoreResult({ histKey, currKey }, scoreResult, threadIdx);
	return true;
}
