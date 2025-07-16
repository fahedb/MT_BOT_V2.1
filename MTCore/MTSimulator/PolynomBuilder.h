#include <vector>
#include "GraphStructres.h"
#include<cmath>
#include "LOGGER.H"
#include "Polynom.h"

#pragma once

/// <summary>
/// Class for building a polynomial from a set of points in a graph structure.
/// </summary>

// Author: Fahed Elbatash
// Date: 2025-07-12
// This class provides methods to build a polynomial using the Vandermonde matrix method
// and solve the resulting linear system using Gaussian elimination.
// The polynomial is represented as a vector of coefficients, where the index corresponds to the power of the variable.
// The coefficients are in descending order of power, i.e., the first element is the coefficient for the highest power.

class PolynomBuilder
{
public:
	// Builds a polynomial from a set of points in the graph structure
	static Polynom BuildAPolynom(GraphStructure& graph);
private:
	// Builds a polynomial using the Vandermonde matrix method
    static std::vector<double> BuildPolynomialVector(std::vector<std::pair<double, double>>& points);

	// Solves the linear system Ax = b using Gaussian elimination
    static bool SolveLinearSystem(std::vector<std::vector<double>>& A, std::vector<double>& b, std::vector<double>& x);

};