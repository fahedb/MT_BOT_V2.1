#pragma once
#include "IndicatorEnums.h"
#include <map>
#include <unordered_map>
#include "robin_hood.h"

struct IndicatorScore {


	IndicatorScore();
	// Calculate the total score
	void calcTotal();
	void Reset() { // Clear the scores and total
		for(int i =0; i < BAR_PERIODS_COUNT; i++) {
			m_scores[i] = 0.0; // Reset each score to 0.0
		}
	}
	int getSize() const { return m_size; }
	double Get_Score(BarPeriod barP) const { 
		if (barP == BarPeriod::NONE) return 0.0; // Handle NONE period case
		return m_scores[static_cast<int>(barP)]; 
	}
	// Overload the + operator for IndicatorScore
	IndicatorScore operator+(const IndicatorScore& other) const;
	// += operator
	IndicatorScore& operator+=(const IndicatorScore& other);
	// - operatpr
	IndicatorScore operator-(const IndicatorScore& other) const;
	// / operator to divide IndicatorScore by a other IndicatorScore
	IndicatorScore operator/(const IndicatorScore& other) const;

	double m_scores[BAR_PERIODS_COUNT] = { 0 }; // Scores for each length
	int m_size = BAR_PERIODS_COUNT;
	int total=0;
};
IndicatorScore operator*(const IndicatorScore& score, double factor); // External Multiply operator
IndicatorScore operator*(double factor, const IndicatorScore& score); // External Multiply operator

struct ScoresResult {


	ScoresResult();
	// Calculates and updates the total score based on other scores
	void calcTotalResult();
	void Reset();

	// + operator to combine two ScoresResult objects
	ScoresResult operator+(const ScoresResult& other) const; 
	// += operator to combine two ScoresResult objects
	ScoresResult& operator+=(const ScoresResult& other);
	// - operator to subtract two ScoresResult objects
	ScoresResult operator-(const ScoresResult& other) const;
	// / operator to divide ScoresResult by another ScoresResult
	ScoresResult operator/(const ScoresResult& other) const;
	std::vector<IndicatorScore> indicatorScores;
	IndicatorScore totalScore;
	void SetLockedResult(bool locked) { m_lockedResult = locked; }
private:
	// Helper function to combine scores into totalScore
	bool m_lockedResult = false;	
	IndicatorScore combineScores() const;
};
ScoresResult operator*(const ScoresResult& score, double factor); // External Multiply ScoresResult by a factor
ScoresResult operator*(double factor, const ScoresResult& score); // External Multiply ScoresResult by a factor



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
	IndicatorValues() 
	{
		BarPeriodList barPeriods;
		values.assign(barPeriods.periods.size(), 0.0); // Initialize the vector with zeros for each BarPeriod
		slopes.assign(barPeriods.periods.size(), 0.0); // Initialize the vector with zeros for each BarPeriod
	}
	void Set_SingleSlope(BarPeriod barP, double slope) { slopes[static_cast<int>(barP)] = slope; } // Set the slope for the specified BarPeriod
	void Set_SingleValue(BarPeriod barP, double val) { values[static_cast<int>(barP)] = val; }// Set the value for the specified BarPeriod
	
	void Set_Value(IndicatorValueType valueType, BarPeriod barP, double val);
	
	double Get_SingleValue(BarPeriod barP) const { return values.at(static_cast<int>(barP)); } // Get the value for the specified BarPeriod
	double Get_SingleSlope(BarPeriod barP) const { return slopes.at(static_cast<int>(barP)); } // Get the slope for the specified BarPeriod

	double Get_Value(IndicatorValueType valueType, BarPeriod barP) const // General function to get value or slope based on IndicatorValueType
	{
		if (valueType == IndicatorValueType::Value)
			return Get_SingleValue(barP);
		else if (valueType == IndicatorValueType::Slope)
			return Get_SingleSlope(barP);
		return 0.0; // Default case, should not happen
	}
private:
	// Indicator Values
	std::vector<double> values; // Values for each length
	std::vector<double> slopes; // Slopes for each length

};


class BaseIndicator
{
public:
	virtual void SetValues(IndicatorValues curr, IndicatorValues prev);
	BaseIndicator(IndicatorValues curr);
	IndicatorValues& Get_IndicatorValues() { return m_curr_BarValues; }
	IndicatorValues* Get_IndicatorValues_Pointer() { return &m_curr_BarValues; }
	virtual IndicatorType Get_IndicatorType() const = 0; // Pure virtual function to get the indicator type

protected:
	// Indicator values in all periods for current minute
	IndicatorValues m_curr_BarValues;
};

class Volume_Ind : public BaseIndicator
{
public:
	Volume_Ind(IndicatorValues curr) : BaseIndicator(curr) {};
	virtual IndicatorType Get_IndicatorType() const override { return IndicatorType::VOLUME; } // Return the indicator type
};

class RSI_Ind : public BaseIndicator
{
public:
	RSI_Ind(IndicatorValues curr) : BaseIndicator(curr) {}
	virtual IndicatorType Get_IndicatorType() const override { return IndicatorType::RSI; } // Return the indicator type

};

class EMA_Ind :public BaseIndicator
{
public:
	EMA_Ind(IndicatorValues curr) : BaseIndicator(curr) {}
	virtual IndicatorType Get_IndicatorType() const override { return IndicatorType::EMA; } // Return the indicator type
};

class ZLSMA_Ind :public BaseIndicator
{
public:
	ZLSMA_Ind(IndicatorValues curr) : BaseIndicator(curr) {}
	virtual IndicatorType Get_IndicatorType() const override { return IndicatorType::ZLSMA; } // Return the indicator type
};


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
		if (barP == BarPeriod::NONE) return 0.0; // Handle NONE period case
		return indValues.Get_Value(valueType, barP); // Get the value or slope based on the valueType
	}
};