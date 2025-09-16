#include "Polynom.h"

#pragma once

// This class is responsible for comparing two polynomials
// Based on Various criteria such as weighted coefficients, YValues, and RMSE (Root Mean Square Error).

class PolynomComparer
{
public:
	// Compare two polynomials based on their coefficients
	static double Compare_ByWeightedCoefficients(const Polynom& p1,const Polynom& p2);

	// Compare two polynomials based on their Y-values at specific points
	static double Compare_ByEval(const Polynom& p1,const Polynom& p2, double fromX = 0.0, double toX = 5.0, int steps = 10);

	// Compare two polynomials using Root Mean Square Error (RMSE)
	static double Compare_ByRMSE(const Polynom& p1, const Polynom& p2, double fromX = 0.0, double toX = 5.0, int steps = 10);

	static double EvaluatePolynom(const Polynom& p, double x); // Evaluate a polynomial at a specific point
};

