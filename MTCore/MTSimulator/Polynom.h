#pragma once

#include <vector>

/// <summary>
/// Polynom class for representing a polynomial.
/// </summary>

/// Author: Fahed Elbatash
/// Date: 2025-07-12
/// /// This class provides methods to set polynomial coefficients and add members to the polynomial at specific powers.

class Polynom
{
private:
	int size;
	std::vector<double> m_polynom; // The index of the vector is the Power, and the value is the coefficient
public:
	void SetPolynom(const std::vector<double>& polynom); // Set the polynomial coefficients
	void AddMemeber(double front, int upper); // Add a member to the polynomial at a specific power
};