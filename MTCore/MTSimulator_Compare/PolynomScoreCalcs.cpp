#include "PolynomScoreCalcs.h"
#include "PolynomTimeLineCache.h"
#include "ScoreResultsCachePolynom.h"
#include "LOGGER.H"
#include "PolynomComparer.h"

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

ScoresResult PolynomScoreCalcs::CompareTimeLines(std::vector<ChangeNode*>* histTL, std::vector<ChangeNode*>* currTL,int threadIdx, int& resCacheHits, int& resCacheMisses, int& polyCacheHits, int& polyCacheMisses, int maxScorePossibleForPolynom,int totalScoreToAct)
{
	if (histTL->empty() || currTL->empty())
		return ScoresResult(); // Return an empty ScoresResult if either vector is empty
	if (histTL->size() != currTL->size())
	{
		Logger::getInstance().WriteLog("Vectors are not of the same size: " + std::to_string(histTL->size()) + " vs " + std::to_string(currTL->size()), false, true, LogType::ERR);
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

	

	for (int i = 0; i < histTL->size(); i++)
	{
		if (i + polyLen >= histTL->size() || i + polyLen >= currTL->size())
			break; // Break if we exceed the size of either vector

		int remainingCalcs = (histTL->size() - i) - polyLen + 1; // Calculate remaining calculations
		if (IsEarlyExitAllowed(maxScorePossibleForPolynom, totalScore.totalScore.total, remainingCalcs, totalScoreToAct))
			break; // Early exit if further calculations cannot affect the result

		ScoresResult* foundRes = nullptr; // Initialize foundRes to nullptr
		if (scoreCachePolicy->EnableScoreCaching)
			foundRes = TryFindCachedScore(PolynomTimeLineKey(histTL->at(i)->point_DateTimeLong, histTL->at(i + polyLen - 1)->point_DateTimeLong),
				PolynomTimeLineKey(currTL->at(i)->point_DateTimeLong, currTL->at(i + polyLen - 1)->point_DateTimeLong)
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
			histTL->begin() + (static_cast<size_t>(i)),
			histTL->begin() + (static_cast<size_t>(i) + static_cast<size_t>(polyLen)));

		std::vector<ChangeNode*> partialVec2(
			currTL->begin() + (static_cast<size_t>(i)),
			currTL->begin() + (static_cast<size_t>(i) + static_cast<size_t>(polyLen)));

		// PolynomTimeLine objects for the current segments
		PolynomTimeLineKey histKey{ partialVec1.front()->point_DateTimeLong,
										  partialVec1.back()->point_DateTimeLong };
		PolynomTimeLineKey currKey{ partialVec2.front()->point_DateTimeLong,
										  partialVec2.back()->point_DateTimeLong };

		// Get from cache (can be nullptr)
		PolynomTimeLine* histPoly = TryFindPolynomInCache(histKey, CacheType::HISTORICAL);
		PolynomTimeLine* currPoly = TryFindPolynomInCache(currKey, CacheType::CURRENT);

		// if not found in singleton cache, build it locally
		std::unique_ptr<PolynomTimeLine> histLocal;
		std::unique_ptr<PolynomTimeLine> currLocal;

		if (!histPoly) histLocal = std::make_unique<PolynomTimeLine>(partialVec1);
		if (!currPoly) currLocal = std::make_unique<PolynomTimeLine>(partialVec2);

		PolynomTimeLine* hist = histPoly ? histPoly : histLocal.get();
		PolynomTimeLine* curr = currPoly ? currPoly : currLocal.get();


		TimeLinePolynomScore localScore = CalcScore(hist->Get_PolynomPackage(), curr->Get_PolynomPackage());


		totalScore += *localScore.GetScoreContainer(IndicatorValueType::Value);
		totalScore += *localScore.GetScoreContainer(IndicatorValueType::Slope);

		if (!foundRes)
			TryStoreScoreInCache(partialVec1, partialVec2, localScore, threadIdx); // Try to store the score in the cache

		// Cache the polynomials if they were not from cache
		if (!histPoly)
		{
			totalPolyCacheMisses++;
			histPolyCache.try_emplace(histKey, std::move(*histLocal.get()));
		}
		else
			totalPolyCacheHits++;
		if (!currPoly)
		{
			totalPolyCacheMisses++;
			currPolyCache.try_emplace(currKey, std::move(*currLocal.get()));
		}
		else
			totalPolyCacheHits++;
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

bool PolynomScoreCalcs::IsEarlyExitAllowed(int maxScorePossibleForPolynom,int currScore, int remainingCalcs, int totalScoreToAct)
{
	// logic for early stopping if remaining calculations can't affect the result
	if (maxScorePossibleForPolynom <= 0 || remainingCalcs <= 0)
		return false; // Cannot determine early exit if max score or remaining calculations are non-positive
	if (currScore + (maxScorePossibleForPolynom * remainingCalcs) < totalScoreToAct )
		return true; // Early exit allowed if even the maximum possible score from remaining calculations cannot reach the score to act
	return false; // Otherwise, do not allow early exit
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

PolynomTimeLine* PolynomScoreCalcs::TryFindPolynomInCache(PolynomTimeLineKey& key, CacheType cT)
{
	PolynomTimeLine* foundRes = nullptr; // Initialize foundRes to nullptr
	foundRes = PolynomTimeLineCache::getInstance()->GetPolynomTimeLine(key, cT); // Retrieve the historical polynomial timeline cache
	if (foundRes != nullptr) // If a polynomial timeline is found in the cache
	{
		return foundRes; // Return the found timeline
	}
	return nullptr; // Return nullptr if not found in cache
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
