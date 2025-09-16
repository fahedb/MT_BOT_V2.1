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
	explicit PolynomChangeNodePackage(const std::vector<ChangeNode*>& nodes, PolynomTimeLineCacheType cT)
	{
		m_inCache = 0;
		PolynomMap* cached = nullptr;

		{
			PolynomMap& ref = PolynomChangeNodePackageBuilder::TryFindPolynomInCache(nodes, cT, m_inCache);
			if (m_inCache) cached = std::addressof(ref);
		}

		if (cached) {
			m_mapRef = cached;                       // Non-owning
		}
		else {
			m_owned = std::make_unique<PolynomMap>(PolynomChangeNodePackageBuilder::BuildPolynomForChangeNodes(nodes, cT));
			m_mapRef = m_owned.get();                // owning
		}
	}
	// Copy constructor
	PolynomChangeNodePackage(const PolynomChangeNodePackage& other)
		: m_inCache(other.m_inCache)
	{
		if (other.m_owned) {
			// the source was owning -> create a new owned copy
			m_owned = std::make_unique<PolynomMap>(*other.m_owned);
			m_mapRef = m_owned.get();
		}
		else {
			// the source was non-owning -> just copy the reference
			m_mapRef = other.m_mapRef;
		}
	}	

	// Move constructor
	PolynomChangeNodePackage(PolynomChangeNodePackage&& other) noexcept
		: m_owned(std::move(other.m_owned)),
		m_mapRef(nullptr),
		m_inCache(other.m_inCache)
	{
		m_mapRef = m_owned ? m_owned.get() : other.m_mapRef;
		// Leave other in a valid state
		other.m_mapRef = nullptr;
		other.m_inCache = false;
	}

	PolynomChangeNodePackage(PolynomChangeNodePackage*) = delete;

	~PolynomChangeNodePackage() = default; // Default destructor

	const Polynom* GetPolynom(const PolynomKey& key) const {
		if (!m_mapRef) return nullptr;
		auto it = m_mapRef->find(key);
		return (it == m_mapRef->end()) ? nullptr : &it->second;
	}

	PolynomMap* GetPolynomMap_ptr() { // Get the entire polynomial map 
			return m_mapRef;
	}

	int m_inCache = 0; // Indicates if the polynomials were retrieved from cache (1) or newly computed (0)
private:
	std::unique_ptr<PolynomMap> m_owned; // Owned map if not from cache
	PolynomMap* m_mapRef = nullptr;      // Reference to the map (either owned or from cache)	
};

