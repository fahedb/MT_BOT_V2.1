#include <vector>
#include "ScoreCalcs.h"
#include "PolynomChangeNodePackage.h"
#include "PolynomScore.h"
#include "PolynomComparer.h"

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
    static TimeLinePolynomScore CalcScore(PolynomChangeNodePackage* hist, PolynomChangeNodePackage* curr);
};

