#include <vector>
#include "ScoreCalcs.h"
#include "ChangeNode.h"

#pragma once
class PolynomScoreCalcs
{
    CalcScore(ChangeNode* ChNHistory, ChangeNode* ChNcurr)
    {
        auto histIndPointer = ChNHistory->Get_Indicator_Pointer(IndicatorType::POLYNOMIAL, IndicatorLen::NONE);
        auto currIndPointer = ChNcurr->Get_Indicator_Pointer(IndicatorType::POLYNOMIAL, IndicatorLen::NONE);
        IndicatorScore score;
        if (!histIndPointer || !currIndPointer)
            return score;
        const std::vector<double>& hist = histIndPointer->Get_IndicatorValues().polynomialCoefficients;
        const std::vector<double>& curr = currIndPointer->Get_IndicatorValues().polynomialCoefficients;
        score.PolynomialScore = ScoreBetweenPolynomials(hist, curr);
        return score;
	}

private:
    static double ScoreBetweenPolynomials(const std::vector<double>& a, const std::vector<double>& b) {
        double score = 0;
        for (int i = 0; i < a.size(); ++i) {
			score += RelativeScoreCalcs::CalcScore(a[i], b[i]);
        }
        return 10.0 * score / a.size(); // ρεμν ωμ 0–10
    }
};

