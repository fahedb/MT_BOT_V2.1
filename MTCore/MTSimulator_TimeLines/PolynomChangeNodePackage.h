#include <vector>
#include "ChangeData.h"
#include "PolynomChangeNodePackageBuilder.h"
#include "PolynomKey.h"

#pragma once
class PolynomChangeNodePackage
{
public:
	// Default constructor
	explicit PolynomChangeNodePackage(const std::vector<ChangeNode*>& nodes)
	{
		m_owned = std::make_unique<PolynomMap>(PolynomChangeNodePackageBuilder::BuildPolynomForChangeNodes(nodes));
	}
	// Copy constructor
	PolynomChangeNodePackage(const PolynomChangeNodePackage& other)
	{
		if (other.m_owned) {
			// the source was owning -> create a new owned copy
			m_owned = std::make_unique<PolynomMap>(*other.m_owned);
		}
	}	

	// Move constructor
	PolynomChangeNodePackage(PolynomChangeNodePackage&& other) noexcept
		: m_owned(std::move(other.m_owned))
	{
	}

	PolynomChangeNodePackage(PolynomChangeNodePackage*) = delete;

	~PolynomChangeNodePackage() = default; // Default destructor

	const Polynom* GetPolynom(const PolynomKey& key) const {
		if (!m_owned) return nullptr;
		auto it = m_owned->find(key);
		return (it == m_owned->end()) ? nullptr : &it->second;
	}

	PolynomMap* GetPolynomMap_ptr() { // Get the entire polynomial map 
			return m_owned.get();
	}

private:
	std::unique_ptr<PolynomMap> m_owned; // Owned map if not from cache
};

