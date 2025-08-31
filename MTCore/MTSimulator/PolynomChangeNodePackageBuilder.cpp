#include "PolynomChangeNodePackageBuilder.h"
#include "PolynomTimeLine.h"
#include "PolynomTimeLineCache.h"

PolynomTimeLineCache* m_cache = nullptr; // Initialize the cache pointer to nullptr

const PolynomMap& TryFindInCache(PolynomTimeLineKey PolynomKey)
{
	PolynomTimeLine* foundRes = nullptr; // Initialize foundRes to nullptr

	if (m_cache == nullptr) // If the cache is not initialized
	{
		m_cache = PolynomTimeLineCache::getInstance(); // Get the singleton instance of PolynomTimeLineCache
	}

	foundRes = m_cache->GetPolynomTimeLine(PolynomKey, PolynomTimeLineCacheType::CURRENT); // Retrieve the current polynomial timeline cache
	if (foundRes != nullptr) // If a polynomial timeline is found in the cache
	{
		return foundRes->Get_PolynomPackage()->m_map; // Return the polynomial map from the found timeline
	}

	foundRes = m_cache->GetPolynomTimeLine(PolynomKey, PolynomTimeLineCacheType::HISTORICAL); // Retrieve the historical polynomial timeline cache
	if (foundRes != nullptr) // If a polynomial timeline is found in the cache
	{
		return foundRes->Get_PolynomPackage()->m_map; // Return the polynomial map from the found timeline
	}
	return PolynomMap(); // Return an empty polynomial map if not found in cache
}

PolynomMap PolynomChangeNodePackageBuilder::BuildPolynomForChangeNodes(std::vector<ChangeNode*> nodes)
{
	PolynomTimeLineKey key;

	key.startTimePoint = nodes.front()->point_DateTimeLong; // Set the start time point from the first node
	key.endTimePoint = nodes.back()->point_DateTimeLong; // Set the end time point from the last node
	
	const PolynomMap& cachedPolynomMap = TryFindInCache(key); // Try to find the polynomial in the cache
	if(cachedPolynomMap.size())
	{
		return cachedPolynomMap; // If found in cache, return the cached polynomial map
	}
	PolynomMap cacledPolynomMap;
	IndicatorTypeList indTypes; // List of indicator types
	IndicatorLenList indLengths; // List of indicator lengths
	IndicatorValueTypeList indValueTypes; // List of indicator value types (Value or Slope)

	// Build polynomials for different indicators and lengths
	for (const auto& type : indTypes.types) // Iterate through each indicator type
	{
		for(const auto& len : indLengths.lengths) // Iterate through each indicator length
		{
			for(const auto& valueType : indValueTypes.valueTypes) // Iterate through each value type (Value or Slore)
			{
				cacledPolynomMap[PolynomKey(type, len, valueType)] = BuildPolynomForIndicator(nodes, type, len, valueType); // Build polynomial for the given type, length, and value type
			}
		}
	}

	return cacledPolynomMap;
}
