#include "PolynomChangeNodePackageBuilder.h"
#include "PolynomTimeLine.h"
#include "PolynomTimeLineCache.h"


IndicatorTypeList m_glob_indTypes; // List of indicator types
IndicatorLenList m_glob_indLengths; // List of indicator lengths
IndicatorValueTypeList m_glob_indValueTypes; // List of indicator value types (Value or Slope)
PolynomTimeLineCache* m_cache = nullptr; // Initialize the cache pointer to nullptr

PolynomMap* TryFindInCache(PolynomTimeLineKey PolynomKey, PolynomTimeLineCacheType cT)
{
	PolynomTimeLine* foundRes = nullptr; // Initialize foundRes to nullptr

	if (cT == PolynomTimeLineCacheType::CURRENT)
		foundRes = m_cache->GetPolynomTimeLine(PolynomKey, PolynomTimeLineCacheType::CURRENT); // Retrieve the current polynomial timeline cache
	else
		foundRes = m_cache->GetPolynomTimeLine(PolynomKey, PolynomTimeLineCacheType::HISTORICAL); // Retrieve the historical polynomial timeline cache

	if (foundRes != nullptr) // If a polynomial timeline is found in the cache
	{
		return &foundRes->Get_PolynomPackage()->m_map; // Return the polynomial map from the found timeline
	}
	return nullptr; // Return an empty polynomial map if not found in cache
}


PolynomMap PolynomChangeNodePackageBuilder::BuildPolynomForChangeNodes(std::vector<ChangeNode*> nodes, PolynomTimeLineCacheType cT,int& inCache)
{
	PolynomTimeLineKey key;

	key.startTimePoint = nodes.front()->point_DateTimeLong; // Set the start time point from the first node
	key.endTimePoint = nodes.back()->point_DateTimeLong; // Set the end time point from the last node
	
	PolynomMap* cachedPolynomMap = TryFindInCache(key,cT); // Try to find the polynomial in the cache
	if(cachedPolynomMap)
	{
		inCache = 1;
		return *cachedPolynomMap; // If found in cache, return the cached polynomial map
	}
	
	inCache = 0;
	
	PolynomMap cacledPolynomMap;

	// Build polynomials for different indicators and lengths
	for (const auto& type : m_glob_indTypes.types) // Iterate through each indicator type
	{
		for(const auto& len : m_glob_indLengths.lengths) // Iterate through each indicator length
		{
			for(const auto& valueType : m_glob_indValueTypes.valueTypes) // Iterate through each value type (Value or Slore)
			{
				cacledPolynomMap[PolynomKey(type, len, valueType)] = BuildPolynomForIndicator(nodes, type, len, valueType); // Build polynomial for the given type, length, and value type
			}
		}
	}
	
	return cacledPolynomMap;
}

void PolynomChangeNodePackageBuilder::Set_Cache()
{
	if (m_cache == nullptr) // If the cache is not already initialized
	{
		m_cache = PolynomTimeLineCache::getInstance(); // Create a new instance of PolynomTimeLineCache
	}
}
