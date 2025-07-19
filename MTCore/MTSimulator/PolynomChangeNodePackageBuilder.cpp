#include "PolynomChangeNodePackageBuilder.h"

PolynomMap PolynomChangeNodePackageBuilder::BuildPolynomForChangeNodes(std::vector<ChangeNode*> nodes)
{
	PolynomMap polynomMap;
	// Build polynomials for different indicators and lengths

	// EMA Indicators: Values
	polynomMap[PolynomKey(IndicatorType::EMA, IndicatorLen::L10, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L10, IndicatorValueType::Value);
	polynomMap[PolynomKey(IndicatorType::EMA, IndicatorLen::L15, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L15, IndicatorValueType::Value);
	polynomMap[PolynomKey(IndicatorType::EMA, IndicatorLen::L20, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L20, IndicatorValueType::Value);
	polynomMap[PolynomKey(IndicatorType::EMA, IndicatorLen::L25, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L25, IndicatorValueType::Value);
	polynomMap[PolynomKey(IndicatorType::EMA, IndicatorLen::L30, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L30, IndicatorValueType::Value);

	// EMA Indicators: Slopes
	polynomMap[PolynomKey(IndicatorType::EMA, IndicatorLen::L10, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L10, IndicatorValueType::Slope);
	polynomMap[PolynomKey(IndicatorType::EMA, IndicatorLen::L15, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L15, IndicatorValueType::Slope);
	polynomMap[PolynomKey(IndicatorType::EMA, IndicatorLen::L20, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L20, IndicatorValueType::Slope);
	polynomMap[PolynomKey(IndicatorType::EMA, IndicatorLen::L25, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L25, IndicatorValueType::Slope);
	polynomMap[PolynomKey(IndicatorType::EMA, IndicatorLen::L30, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::EMA, IndicatorLen::L30, IndicatorValueType::Slope);

	// RSI Indicators: Values
	polynomMap[PolynomKey(IndicatorType::RSI, IndicatorLen::L10, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L10, IndicatorValueType::Value);
	polynomMap[PolynomKey(IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Value);
	polynomMap[PolynomKey(IndicatorType::RSI, IndicatorLen::L20, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L20, IndicatorValueType::Value);
	polynomMap[PolynomKey(IndicatorType::RSI, IndicatorLen::L25, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L25, IndicatorValueType::Value);
	polynomMap[PolynomKey(IndicatorType::RSI, IndicatorLen::L30, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L30, IndicatorValueType::Value);


	// RSI Indicators: Slopes
	polynomMap[PolynomKey(IndicatorType::RSI, IndicatorLen::L10, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L10, IndicatorValueType::Slope);
	polynomMap[PolynomKey(IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L15, IndicatorValueType::Slope);
	polynomMap[PolynomKey(IndicatorType::RSI, IndicatorLen::L20, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L20, IndicatorValueType::Slope);
	polynomMap[PolynomKey(IndicatorType::RSI, IndicatorLen::L25, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L25, IndicatorValueType::Slope);
	polynomMap[PolynomKey(IndicatorType::RSI, IndicatorLen::L30, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::RSI, IndicatorLen::L30, IndicatorValueType::Slope);

	// ZLSMA Indicators: Values
	polynomMap[PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L10, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L10, IndicatorValueType::Value);
	polynomMap[PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L15, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L15, IndicatorValueType::Value);
	polynomMap[PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L20, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L20, IndicatorValueType::Value);
	polynomMap[PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L25, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L25, IndicatorValueType::Value);
	polynomMap[PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L30, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L30, IndicatorValueType::Value);

	// ZLSMA Indicators: Slopes
	polynomMap[PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L10, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L10, IndicatorValueType::Slope);
	polynomMap[PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L15, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L15, IndicatorValueType::Slope);
	polynomMap[PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L20, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L20, IndicatorValueType::Slope);
	polynomMap[PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L25, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L25, IndicatorValueType::Slope);
	polynomMap[PolynomKey(IndicatorType::ZLSMA, IndicatorLen::L30, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::ZLSMA, IndicatorLen::L30, IndicatorValueType::Slope);
	
	// VOLUME Indicators: Values
	polynomMap[PolynomKey(IndicatorType::VOLUME, IndicatorLen::NONE, IndicatorValueType::Value)] = BuildPolynomForIndicator(nodes, IndicatorType::VOLUME, IndicatorLen::L10, IndicatorValueType::Value);
	// VOLUME Indicators: Slopes
	polynomMap[PolynomKey(IndicatorType::VOLUME, IndicatorLen::NONE, IndicatorValueType::Slope)] = BuildPolynomForIndicator(nodes, IndicatorType::VOLUME, IndicatorLen::L10, IndicatorValueType::Slope);
	return polynomMap;
}
