#include <vector>
#include "ChangeData.h"
#include "PolynomChangeNodePackageBuilder.h"
#include "CONFIGRATIONS.H"
#include "PolynomKey.h"

#pragma once
class PolynomChangeNodePackage
{
public:
	// Default constructor
	PolynomChangeNodePackage(const std::vector<ChangeNode*>& nodes, PolynomTimeLineCacheType cT)
	{
		m_map = PolynomChangeNodePackageBuilder::BuildPolynomForChangeNodes(nodes, cT, m_inCache);
	}
	// Copy constructor
	PolynomChangeNodePackage(const PolynomChangeNodePackage& other)
	{
		m_map = other.m_map; // Copy the map
	}

	// Move constructor
	PolynomChangeNodePackage(PolynomChangeNodePackage&& other) noexcept
	{
		m_map = std::move(other.m_map);
	}

	PolynomChangeNodePackage(PolynomChangeNodePackage* other) noexcept
	{
		m_map = std::move(other->m_map); // Move the map from the other object
	}
	~PolynomChangeNodePackage()
	{
		m_map.clear(); // Clear the map
	}
	PolynomMap m_map;
	int m_inCache = 0; // Indicates if the polynomials were retrieved from cache (1) or newly computed (0)
	Polynom* GetPolynom(const PolynomKey& key)
	{
		auto it = m_map.find(key);
		if (it != m_map.end())
			return &it->second; // Return the found Polynom
		return nullptr; // Return an empty Polynom if not found
	}
private:
	
};

