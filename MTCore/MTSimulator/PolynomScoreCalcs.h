#include <vector>
#include "ScoreCalcs.h"
#include "PolynomChangeNodePackage.h"

#pragma once

struct PolynomScore {
    double L10=0;
    double L15=0;
	double L20=0;
    double L25=0;
    double L30=0;

    double L10_slope=0;
	double L15_slope=0;
    double L20_slope=0;
    double L25_slope=0;
    double L30_slope=0;
	double totalPolynomialScore=0;

    void CalculateTotalScore() {
        totalPolynomialScore = (L10 + L15 + L20 + L25 + L30 +
                                L10_slope + L15_slope + L20_slope +
                                L25_slope + L30_slope); // Average score
	}
};

class PolynomScoreCalcs
{
    PolynomScore CalcScore(PolynomChangeNodePackage* hist, PolynomChangeNodePackage* curr);

    double CalcSinglePolynomScore(Polynom* hist,Polynom* curr, const int weight = 10) {
        if (!hist || !curr || hist->GetSize() != curr->GetSize()) {
            return 0.0; // Return 0 if polynomials are not valid or sizes do not match
        }
        return ScoreBetweenPolynomials(hist->GetCoefficients(), curr->GetCoefficients(), weight);
	}

private:
    static double ScoreBetweenPolynomials(const std::vector<double>& a, const std::vector<double>& b,const int weight = 10) {
        double score = 0;
        for (int i = 0; i < a.size(); ++i) {
			score += RelativeScoreCalcs::CalcScore(a[i], b[i]);
        }
		return weight * score / a.size(); // Weighting the score by the number of coefficients
    }
};

