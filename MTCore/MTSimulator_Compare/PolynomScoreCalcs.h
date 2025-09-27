#include <vector>
#include "PolynomChangeNodePackage.h"
#include "PolynomScore.h"
#include "..\MyRobinHoodMap\HashingStructs.h"
#include "PolynomTimeLine.h"
#include "CacheEnums.h"

#pragma once

/// <summary>
/// This class is responsible for calculating the score of polynomials based on their historical and current values.
/// It uses the PolynomComparer to compare the polynomials and calculate a score based on various metrics.
/// Author: Fahed Elbatash
/// Date: 2025-07-29
/// </summary>

class PolynomScoreCalcs
{
public:
    static ScoresResult CompareTimeLines(std::vector<ChangeNode*>* vec1, std::vector<ChangeNode*>* vec2,int threadIdx, int& resCacheHits, int& resCacheMisses, int& polyCacheHits, int& polyCacheMisses);
private:
    static TimeLinePolynomScore CalcScore(PolynomChangeNodePackage* hist, PolynomChangeNodePackage* curr);
    static ScoresResult* TryFindCachedScore(const PolynomTimeLineKey& histKey, const PolynomTimeLineKey& currKey, int threadIdx);
    static PolynomTimeLine* TryFindPolynomInCache(const PolynomTimeLineKey& key, CacheType cT);
    static bool TryStoreScoreInCache(const std::vector<ChangeNode*>& histSegment, const std::vector<ChangeNode*>& currSegment, TimeLinePolynomScore& score, int threadIdx);
};

