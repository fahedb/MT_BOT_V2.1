#include "PolynomComparer.h"
#include <cmath>

double PolynomComparer::EvaluatePolynom(Polynom& p, double x) // Optimal evaluation using Horner's method for max performance
{
    const auto& a = p.GetCoefficients();
    const double* beg = a.data();
    const double* end = beg + a.size();

    double r = 0.0;
    while (end != beg) {
        --end;
        r = r * x + *end;
    }
    return r;
}

double PolynomComparer::Compare_ByWeightedCoefficients(Polynom& p1,Polynom& p2)
{
    const auto& c1 = p1.GetCoefficients();
    const auto& c2 = p2.GetCoefficients();
    int maxSize = std::max(c1.size(), c2.size());

    double numerator = 0, denom1 = 0, denom2 = 0;
    for (int i = 0; i < maxSize; ++i)
    {
        double a = (i < c1.size()) ? c1[i] : 0;
        double b = (i < c2.size()) ? c2[i] : 0;
		double weight = static_cast<double>(i + 1); // Weighting by index (1-based)

        numerator += weight * a * b;
        denom1 += weight * a * a;
        denom2 += weight * b * b;
    }

    if (denom1 == 0 || denom2 == 0) return 0;
	return numerator / (std::sqrt(denom1) * std::sqrt(denom2)); // Return cosine similarity between the two polynomials
}

double PolynomComparer::Compare_ByEval(Polynom& p1,Polynom& p2, double fromX, double toX, int steps)
{
    if (steps <= 0 || fromX >= toX)
        return 0;

    double dx = (toX - fromX) / steps;
    double totalSquaredDiff = 0.0;

    for (int i = 0; i <= steps; ++i)
    {
        double x = fromX + i * dx;
        double y1 = EvaluatePolynom(p1, x);
        double y2 = EvaluatePolynom(p2, x);
        double diff = y1 - y2;
        totalSquaredDiff += diff * diff;
    }

    double mse = totalSquaredDiff / (steps + 1);
	return 1.0 / (1.0 + std::sqrt(mse)); // Normalized MSE to a score between 0 and 1
}


double PolynomComparer::Compare_ByRMSE( Polynom& p1,Polynom& p2, double fromX, double toX, int steps)
{
    if (steps <= 0 || fromX >= toX)
        return 0;

    double dx = (toX - fromX) / steps;
    double sumSquared = 0.0;

    for (int i = 0; i <= steps; ++i)
    {
        double x = fromX + i * dx;
        double y1 = EvaluatePolynom(p1, x);
        double y2 = EvaluatePolynom(p2, x);
        double diff = y1 - y2;
        sumSquared += diff * diff;
    }

    double rmse = std::sqrt(sumSquared / (steps + 1));
	return 1.0 / (1.0 + rmse); // Normalized RMSE to a score between 0 and 1
}