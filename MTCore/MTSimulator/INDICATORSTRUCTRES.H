#pragma once
#include "Configrations.h"
#include <map>
#define GRAVITY_MIN_STRONG_LEVEL 70.0

enum struct _Points {
	_Points_DEFUALT = 0,
	Points_EURUSD = 4,
	Points_XAUUSD = 2
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
	int basicScore  =0;
	int gravityScore=0;
	IndicatorScore rsiScore;
	IndicatorScore emaScore;
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
	NONE = -1
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
enum struct IndicatorValueType
{
	Value = 0,
	Slope = 1,
	NONE = -1
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

struct Gravity
{
	int m_TotalTicks = 0;
	int m_UpperTicks = 0;
	int m_lowerTicks = 0;
	double m_GravityStren = 0.0;
	GravityType m_type = GravityType::NOT_INITIALIZED;
public:
	void CalcGravityMembers(double open, double close);

private:
	void setGravityType(double open, double close);
	void calcGravityLevel();

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

class Gravity_Ind
{
public:

	Gravity_Ind(double open, double close, Gravity currGravity);
	Gravity* GetGravity() { return &m_value_M1; }

protected:
	Gravity m_value_M1;
	double m_Slope_M1;
private:
	void Assign_Invalid_Data() {m_Slope_M1 = 0;}
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


class IndicatorMap
{
private:
	std::map<std::pair<IndicatorType, IndicatorLen>, BaseIndicator*> m_map;
	BaseIndicator* Get_IndicatorObject(IndicatorType type, IndicatorValues val); // Getting a pointer of an allocated indicator Object
public:
	IndicatorMap(); // Contructor
	~IndicatorMap()
	{
		if (m_map.size())
		{
			for (auto it = m_map.begin(); it != m_map.end(); ++it) {
				if (it->second != nullptr)
					delete it->second;
			}
		m_map.clear();
		}
	}
	void Set_Indicator(IndicatorType, IndicatorLen, IndicatorValues, bool forceInsert = false); // Function to create a new Indicator object
	void Set_Ind_SingleValue(IndicatorType type, IndicatorLen len,BarPeriod barP,double val)
	{
		Validate_Indicator(type, len);
		BaseIndicator* ind = Get_Indicator(type, len);
		Set_IndValue(ind, barP, val);
	}
	void Set_Ind_SingleSlope(IndicatorType type, IndicatorLen len, BarPeriod barP, double val)
	{
		Validate_Indicator(type, len);
		BaseIndicator* ind = Get_Indicator(type, len);
		Set_IndSlope(ind, barP, val);
	}
	BaseIndicator* Get_Indicator(IndicatorType type, IndicatorLen len);
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
};