#pragma once
#include <vector>

// <summary>
// This file contains enumerations and structures related to indicators used in Indicator types, lens and barPeriods
// Author: Fahed Elbatash
// Date: 2025-07-31
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

// <IndicatorType>
// This enum defines the types of indicators used in the system.
enum struct IndicatorType
{
	EMA = 0,
	RSI = 1,
	ZLSMA = 2,
	VOLUME = 3, // currently not used
	NONE = -1
};

// <IndicatorTypeList>
// This struct contains a list of available indicator types.
struct IndicatorTypeList
{
	std::vector<IndicatorType> types = { IndicatorType::EMA, IndicatorType::RSI, IndicatorType::ZLSMA };
};

// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

// <IndicatorValueType>
// This enum defines the types of values that indicators can have, such as Value and Slope.
enum struct IndicatorValueType
{
	Value = 0,
	Slope = 1,
	NONE = -1
};
// <IndicatorValueTypeList>
// This struct contains a list of available indicator value types.
struct IndicatorValueTypeList
{
	std::vector<IndicatorValueType> valueTypes = { IndicatorValueType::Value, IndicatorValueType::Slope };
};
// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

// <BarPeriod>
// This enum defines the different bar periods used in the system, such as M1, M5, M15, etc.
enum class BarPeriod {
	M1 = 0,
	M5 = 1,
	M15 = 2,
	M30 = 3,
	H1 = 4,
	H4 = 5,
	NONE = -1
};
// <BarPeriodList>
// This struct contains a list of available bar periods.
struct BarPeriodList
{
	std::vector<BarPeriod> periods = { BarPeriod::M1, BarPeriod::M5, BarPeriod::M15, BarPeriod::M30, BarPeriod::H1, BarPeriod::H4 };
};
// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

// <IndicatorLen>
// This enum defines the lengths of indicators, such as L10, L15, L20, etc.
enum struct IndicatorLen
{
	L10 = 0,
	L15 = 1,
	L20 = 2,
	L25 = 3,
	L30 = 4,
	NONE = -1
};

// <IndicatorLenList>
// This struct contains a list of available indicator lengths.
struct IndicatorLenList
{
	std::vector<IndicatorLen> lengths = { IndicatorLen::L10, IndicatorLen::L15, IndicatorLen::L20, IndicatorLen::L25, IndicatorLen::L30 };
};
// ------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------

enum struct _Points {
	_Points_DEFUALT = 0,
	Points_EURUSD = 4,
	Points_XAUUSD = 2
};