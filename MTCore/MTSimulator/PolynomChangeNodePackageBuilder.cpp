#include "PolynomChangeNodePackageBuilder.h"

robin_hood::unordered_map<PolynomrKey, Polynom> PolynomChangeNodePackageBuilder::BuildPolynomForChangeNodes(std::vector<ChangeNode*> nodes)
{
	robin_hood::unordered_map<PolynomrKey, Polynom> polynomMap;
	// Build polynomials for different indicators and lengths

	// EMA Indicators: Values
	polynomMap[PolynomrKey(IndicatorType::EMA, IndicatorLen::L10, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L10, IndicatorValueType::Value);
	polynomMap[PolynomrKey(IndicatorType::EMA, IndicatorLen::L15, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L15, IndicatorValueType::Value);
	polynomMap[PolynomrKey(IndicatorType::EMA, IndicatorLen::L20, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L20, IndicatorValueType::Value);
	polynomMap[PolynomrKey(IndicatorType::EMA, IndicatorLen::L25, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L25, IndicatorValueType::Value);
	polynomMap[PolynomrKey(IndicatorType::EMA, IndicatorLen::L30, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L30, IndicatorValueType::Value);

	// EMA Indicators: Slopes
	polynomMap[PolynomrKey(IndicatorType::EMA, IndicatorLen::L10, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L10, IndicatorValueType::Slope);
	polynomMap[PolynomrKey(IndicatorType::EMA, IndicatorLen::L15, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L15, IndicatorValueType::Slope);
	polynomMap[PolynomrKey(IndicatorType::EMA, IndicatorLen::L20, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L20, IndicatorValueType::Slope);
	polynomMap[PolynomrKey(IndicatorType::EMA, IndicatorLen::L25, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L25, IndicatorValueType::Slope);
	polynomMap[PolynomrKey(IndicatorType::EMA, IndicatorLen::L30, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L30, IndicatorValueType::Slope);

	// RSI Indicators: Values
	polynomMap[PolynomrKey(IndicatorType::RSI, IndicatorLen::L10, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L10, IndicatorValueType::Value);
	polynomMap[PolynomrKey(IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Value);
	polynomMap[PolynomrKey(IndicatorType::RSI, IndicatorLen::L20, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L20, IndicatorValueType::Value);
	polynomMap[PolynomrKey(IndicatorType::RSI, IndicatorLen::L25, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L25, IndicatorValueType::Value);
	polynomMap[PolynomrKey(IndicatorType::RSI, IndicatorLen::L30, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L30, IndicatorValueType::Value);


	// RSI Indicators: Slopes
	polynomMap[PolynomrKey(IndicatorType::RSI, IndicatorLen::L10, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L10, IndicatorValueType::Slope);
	polynomMap[PolynomrKey(IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Slope);
	polynomMap[PolynomrKey(IndicatorType::RSI, IndicatorLen::L20, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L20, IndicatorValueType::Slope);
	polynomMap[PolynomrKey(IndicatorType::RSI, IndicatorLen::L25, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L25, IndicatorValueType::Slope);
	polynomMap[PolynomrKey(IndicatorType::RSI, IndicatorLen::L30, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L30, IndicatorValueType::Slope);

	// ZLSMA Indicators: Values
	polynomMap[PolynomrKey(IndicatorType::ZLSMA, IndicatorLen::L10, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L10, IndicatorValueType::Value);
	polynomMap[PolynomrKey(IndicatorType::ZLSMA, IndicatorLen::L15, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L15, IndicatorValueType::Value);
	polynomMap[PolynomrKey(IndicatorType::ZLSMA, IndicatorLen::L20, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L20, IndicatorValueType::Value);
	polynomMap[PolynomrKey(IndicatorType::ZLSMA, IndicatorLen::L25, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L25, IndicatorValueType::Value);
	polynomMap[PolynomrKey(IndicatorType::ZLSMA, IndicatorLen::L30, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L30, IndicatorValueType::Value);

	// ZLSMA Indicators: Slopes
	polynomMap[PolynomrKey(IndicatorType::ZLSMA, IndicatorLen::L10, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L10, IndicatorValueType::Slope);
	polynomMap[PolynomrKey(IndicatorType::ZLSMA, IndicatorLen::L15, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L15, IndicatorValueType::Slope);
	polynomMap[PolynomrKey(IndicatorType::ZLSMA, IndicatorLen::L20, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L20, IndicatorValueType::Slope);
	polynomMap[PolynomrKey(IndicatorType::ZLSMA, IndicatorLen::L25, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L25, IndicatorValueType::Slope);
	polynomMap[PolynomrKey(IndicatorType::ZLSMA, IndicatorLen::L30, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L30, IndicatorValueType::Slope);
	
	// VOLUME Indicators: Values
	polynomMap[PolynomrKey(IndicatorType::VOLUME, IndicatorLen::NONE, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::VOLUME, IndicatorLen::L10, IndicatorValueType::Value);
	// VOLUME Indicators: Slopes
	polynomMap[PolynomrKey(IndicatorType::VOLUME, IndicatorLen::NONE, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::VOLUME, IndicatorLen::L10, IndicatorValueType::Slope);
	return polynomMap;
}
