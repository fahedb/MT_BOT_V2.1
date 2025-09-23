#include "PolynomBuilder.h"
#include "ChangeData.h"
#include "robin_hood.h"
#include "GraphUtils.h"
//#include "PolynomChangeNodePackage.h"
#include "PolynomKey.h"
#include "CacheEnums.h"

#pragma once
typedef robin_hood::unordered_map<PolynomKey, Polynom> PolynomMap;
class PolynomChangeNodePackageBuilder
{
public:
	static PolynomMap& TryFindPolynomInCache(std::vector<ChangeNode*> nodes, CacheType cT, int& inCache);
	static PolynomMap BuildPolynomForChangeNodes(std::vector<ChangeNode*> nodes, CacheType cT);
	static void Set_Cache();
private:
		static Polynom BuildPolynomForIndicator(std::vector<ChangeNode*> nodes,IndicatorType indT,IndicatorLen indL,IndicatorValueType indVT,BarPeriod indP = BarPeriod::M1)
	{
		GraphStructure graph = GraphUtils::Build_NormalizedGraph(nodes, indT, indL, indP,indVT);
		Polynom polynom = PolynomBuilder::BuildAPolynom(graph);
		return polynom;
	}
};

