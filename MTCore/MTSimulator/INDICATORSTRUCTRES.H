#pragma once
#include "LOGGER.H"
#include "Configrations.h"
#include <map>
#include <unordered_map>
#include <array>

enum struct _Points {
	_Points_DEFUALT = 0,
	Points_EURUSD = 4,
	Points_XAUUSD = 2
};

enum class BarPeriod {
	M1 = 1,
	M5 = 5,
	M15 = 15,
	M30 = 30,
	H1 = 60,
	H4 = 240,
	NONE = -1
};

struct IndicatorScore {
	int M1=0;
	int M5=0;
	int M15=0;
	int M30=0;
	int H1=0;
	int H4=0;
	int total=0;
	IndicatorScore();
	// Calculate the total score
	void calcTotal();

	// Overload the + operator for IndicatorScore
	IndicatorScore operator+(const IndicatorScore& other) const;
	IndicatorScore& operator+=(const IndicatorScore& other);
};

struct ScoresResult {
	IndicatorScore rsiScore;
	IndicatorScore zlsmaScore;
	IndicatorScore emaScore;
	IndicatorScore volumeScore;
	IndicatorScore totalScore;
	ScoresResult();
	// Calculates and updates the total score based on other scores
	void calcTotalResult();
	// Overload the + operator for ScoresResult
	ScoresResult operator+(const ScoresResult& other) const;
	ScoresResult& operator+=(const ScoresResult& other);
private:
	// Helper function to combine scores into totalScore
	IndicatorScore combineScores() const;
};

enum struct IndicatorType
{
	EMA = 0,
	RSI = 1,
	ZLSMA = 2,
	VOLUME = 3, // currently not used
	NONE = -1
};
struct IndicatorTypeList
{
	std::array<IndicatorType, 4> types = { IndicatorType::EMA, IndicatorType::RSI, IndicatorType::ZLSMA };
};

enum struct IndicatorValueType
{
	Value = 0,
	Slope = 1,
	NONE = -1
};
struct IndicatorValueTypeList
{
	std::array<IndicatorValueType, 2> valueTypes = { IndicatorValueType::Value, IndicatorValueType::Slope };
};

struct BarPeriodList
{
	std::array<BarPeriod, 6> periods = { BarPeriod::M1, BarPeriod::M5, BarPeriod::M15, BarPeriod::M30, BarPeriod::H1, BarPeriod::H4 };
};

enum struct IndicatorLen
{
	L10 = 10,
	L15 = 15,
	L20 = 20,
	L25=25,
	L30=30,
	NONE = -1
};

struct IndicatorLenList
{
	std::array<IndicatorLen, 5> lengths = { IndicatorLen::L10, IndicatorLen::L15, IndicatorLen::L20, IndicatorLen::L25, IndicatorLen::L30 };
};



enum GravityType
{
	NOT_INITIALIZED = 0,
	Random_Desc = 1,
	Random_Asc = 2,
	Low_Desc = 3,
	High_Desc = 4,
	Low_Asc = 5,
	High_Asc = 6
};

struct IndicatorValues {
	void Set_SingleSlope(BarPeriod barP, double slope);
	void Set_SingleValue(BarPeriod barP, double val);
	// Indicator Values
	double M1;
	double M5;
	double M15;
	double M30;
	double H1;
	double H4;

	// Slopes
	double m_Slope_M1;
	double m_Slope_M5;
	double m_Slope_M15;
	double m_Slope_M30;
	double m_Slope_H1;
	double m_Slope_H4;
};


class BaseIndicator
{
public:
	virtual void SetValues(IndicatorValues curr, IndicatorValues prev);
	BaseIndicator(IndicatorValues curr);
	IndicatorValues Get_IndicatorValues() { return m_curr_BarValues; }
	IndicatorValues* Get_IndicatorValues_Pointer() { return &m_curr_BarValues; }

protected:
	// Indicator values in all periods for current minute
	IndicatorValues m_curr_BarValues;
};

class Volume_Ind : public BaseIndicator
{
public:
	Volume_Ind(IndicatorValues curr) : BaseIndicator(curr) {};
};

class RSI_Ind : public BaseIndicator
{
public:
	RSI_Ind(IndicatorValues curr) : BaseIndicator(curr) {}

};

class EMA_Ind :public BaseIndicator
{
public:
	EMA_Ind(IndicatorValues curr) : BaseIndicator(curr) {}
};

class ZLSMA_Ind :public BaseIndicator
{
public:
	ZLSMA_Ind(IndicatorValues curr) : BaseIndicator(curr) {}
};

#define INDICATOR_TYPES_VEC_LEN 5
#define INDICATOR_LEN_VEC_LEN 5

class IndicatorMap
{
private:
	std::vector<std::vector< BaseIndicator*>> m_vec;

	BaseIndicator* Get_IndicatorObject(IndicatorType type, IndicatorValues val); // Getting a pointer of an allocated indicator Object
public:
	IndicatorMap(); // Contructor
	~IndicatorMap()
	{
		if (m_vec.size())
		{
			for (auto& vec : m_vec)
			{
				for (auto& ind : vec)
				{
					delete ind; // Delete each BaseIndicator object
				}
			}
			m_vec.clear();
		}
	}
	void Set_Indicator(IndicatorType, IndicatorLen, IndicatorValues, bool forceInsert = false); // Function to create a new Indicator object
	void Set_Ind_SingleValue(IndicatorType type, IndicatorLen len,BarPeriod barP,double val); // To Set indicator single value - by type, len and baePeriod
	void Set_Ind_SingleSlope(IndicatorType type, IndicatorLen len, BarPeriod barP, double val); // To Set indicator single slope - by type, len and barPeriod
	BaseIndicator* Get_Indicator(IndicatorType type, IndicatorLen len); // Get Indicator ptr
	std::vector<std::vector< BaseIndicator*>>* Get_Indicators_Vec() { return &m_vec; } // Get the vector of indicators
private:
	void Set_IndValue(BaseIndicator*, BarPeriod, double);
	void Set_IndSlope(BaseIndicator*, BarPeriod, double);
	void Validate_Indicator(IndicatorType type, IndicatorLen len); // A function to check if there is an object for the relevant indicator, if not it will creat one and insert it in the map
};


class Indicators_Utils
{
public:

	static IndicatorLen Get_IndicatorLenEnum(int indL)
	{
		if (indL == 10)	return IndicatorLen::L10;
		if (indL == 15)	return IndicatorLen::L15;
		if (indL == 20)	return IndicatorLen::L20;
		if (indL == 25)	return IndicatorLen::L25;
		if (indL == 30)	return IndicatorLen::L30;

		return IndicatorLen::NONE;
	}
	static int Get_LenIndex(IndicatorLen len)
	{
		switch (len)
		{
		case IndicatorLen::L10: return 0;
		case IndicatorLen::L15: return 1;
		case IndicatorLen::L20: return 2;
		case IndicatorLen::L25: return 3;
		case IndicatorLen::L30: return 4;
		case IndicatorLen::NONE: return 0; // NONE is considered as an 0 length
		default:
			return -1; // Invalid length
		}
	}
	static int Get_TypeIndex(IndicatorType type)
	{
		switch (type)
		{
		case IndicatorType::EMA: return 0;
		case IndicatorType::RSI: return 1;
		case IndicatorType::ZLSMA: return 2;
		case IndicatorType::VOLUME: return 3;
		default:
			return -1; // Invalid type
		}
	}

	static double Get_IndicatorValue(const IndicatorValues& indValues, BarPeriod barP, IndicatorValueType valueType)
	{
		switch (barP)
		{
		case BarPeriod::M1:
			return valueType == IndicatorValueType::Value ? indValues.M1 : indValues.m_Slope_M1;
		case BarPeriod::M5:
			return valueType == IndicatorValueType::Value ? indValues.M5 : indValues.m_Slope_M5;
		case BarPeriod::M15:
			return valueType == IndicatorValueType::Value ? indValues.M15 : indValues.m_Slope_M15;
		case BarPeriod::M30:
			return valueType == IndicatorValueType::Value ? indValues.M30 : indValues.m_Slope_M30;
		case BarPeriod::H1:
			return valueType == IndicatorValueType::Value ? indValues.H1 : indValues.m_Slope_H1;
		case BarPeriod::H4:
			return valueType == IndicatorValueType::Value ? indValues.H4 : indValues.m_Slope_H4;
		default:
			return 0.0; // Invalid bar period
		}
	}
};