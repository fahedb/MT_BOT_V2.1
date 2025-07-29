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
	std::vector<double> m_polynom; // The index of the vector is the Power, and the value is the coefficient
public:
	int GetSize() const { return m_polynom.size(); } // Get the size of the polynomial
	void SetPolynom(const std::vector<double>& polynom)
	{
		m_polynom = polynom;
	}
	void AddMember(double front, int upper); // Add a member to the polynomial at a specific power
	double GetCoefficient(int power) const { return (power >= 0 && power < m_polynom.size()) ? m_polynom[power] : 0; } // Get the coefficient for a specific power, returns 0 if power is out of bounds
	std::vector<double> GetCoefficients() const { return m_polynom; } // Get all coefficients of the polynomial
	std::vector<double>* GetCoefficients_ptr() { return &m_polynom; } // Get all coefficients of the polynomial
};