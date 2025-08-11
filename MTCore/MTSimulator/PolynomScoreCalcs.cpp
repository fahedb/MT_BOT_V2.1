#include "PolynomScoreCalcs.h"
#include "PolynomTimeLineCache.h"
#include "ScoreResultsCachePolynom.h"

double CalcSinglePolynomScore(Polynom* hist, Polynom* curr, PolynomComparerPolicy* policy, const int weight = 10) {
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

ScoresResult PolynomScoreCalcs::CompareTimeLines(std::vector<ChangeNode*>* vec1, std::vector<ChangeNode*>* vec2,int threadIdx)
{
	if (vec1->empty() || vec2->empty())
		return ScoresResult(); // Return an empty ScoresResult if either vector is empty
	if (vec1->size() != vec2->size())
	{
		Logger::getInstance().WriteLog("Vectors are not of the same size: " + std::to_string(vec1->size()) + " vs " + std::to_string(vec2->size()), false, true, LogType::ERR);
		return ScoresResult(); // Return an empty ScoresResult if the vectors are not of the same size
	}
	
	const int polyLen = Configrations::getInstance()->Get_Policy<PolynomComparerPolicy>(PolicyType::PolynomComparerPolicy)->PolynomLen_ForScan;
	ScoreResultsCachePolynom* scoreResultsCache = ScoreResultsCachePolynom::getInstance();
	ScoreCachingPolicy* scoreCachePolicy = Configrations::getInstance()->Get_Policy<ScoreCachingPolicy>(PolicyType::ScoreCachingPolicy);

	ScoresResult totalScore;
	for (int i = 0; i < vec1->size(); i += polyLen)
	{
		if (i + polyLen > vec1->size() || i + polyLen > vec2->size())
			break; // Break if we exceed the size of either vector

		std::vector<ChangeNode*> partialVec1(
			vec1->begin() + (static_cast<size_t>(i)),
			vec1->begin() + (static_cast<size_t>(i) + static_cast<size_t>(polyLen)));

		std::vector<ChangeNode*> partialVec2(
			vec2->begin() + (static_cast<size_t>(i)),
			vec2->begin() + (static_cast<size_t>(i) + static_cast<size_t>(polyLen)));

		ScoresResult* foundRes = nullptr; // Initialize foundRes to nullptr
		if (scoreCachePolicy->EnableScoreCaching)
			 foundRes = TryFindCachedScore(partialVec1, partialVec2, threadIdx); // Try to find the score in the cache

		if(foundRes) // If the score was found in the cache
		{
			totalScore += *foundRes; // Add the cached score to the total score
			continue; // Skip to the next iteration if the score was found in the cache
		}
		
		PolynomChangeNodePackage package1(partialVec1); // Create a package for the first vector
		PolynomChangeNodePackage package2(partialVec2); // Create a package for the second vector

		TimeLinePolynomScore localScore = CalcScore(&package1, &package2); // calculate the score for the current polynomials

		totalScore += *localScore.GetScoreContainer(IndicatorValueType::Value);
		totalScore += *localScore.GetScoreContainer(IndicatorValueType::Slope);

		if (!foundRes)
			TryStoreScoreInCache(partialVec1, partialVec2, localScore, threadIdx); // Try to store the score in the cache

	}
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

	IndicatorTypeList indTypes;

	PolynomComparerPolicy* policy = Configrations::getInstance()->Get_Policy<PolynomComparerPolicy>(PolicyType::PolynomComparerPolicy);

	for(const auto& indType : indTypes.types)
	{
		IndicatorLenList indLengths;
		for(const auto& indLen : indLengths.lengths)
		{
			IndicatorValueTypeList indValueTypes;
			for(const auto& valType : indValueTypes.valueTypes)
			{
				auto polynomKey = PolynomKey(indType, indLen, valType);
				score.GetScoreContainer(valType)->indicatorScores.at(indType).scores[BarPeriod::M1] += CalcSinglePolynomScore(hist->GetPolynom(polynomKey), curr->GetPolynom(polynomKey), policy);
			}
		}
	}

	// Calculate the total score
	score.GetScoreContainer(IndicatorValueType::Value)->calcTotalResult();
	score.GetScoreContainer(IndicatorValueType::Slope)->calcTotalResult();
	return score;
}

ScoresResult* PolynomScoreCalcs::TryFindCachedScore(const std::vector<ChangeNode*>& vec1, const std::vector<ChangeNode*>& vec2, int threadIdx)
{
	if (vec1.empty() || vec2.empty())
		return nullptr;

	ScoreResultsCachePolynom* scoreResultsCache = ScoreResultsCachePolynom::getInstance();
	PolynomTimeLineKey histKey(vec1.at(0)->point_DateTimeLong, vec1.back()->point_DateTimeLong);
	PolynomTimeLineKey currKey(vec2.at(0)->point_DateTimeLong, vec2.back()->point_DateTimeLong);

	ScoresResult* foundRes = scoreResultsCache->GetScoreResult(histKey, currKey, threadIdx);
	return foundRes;
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
