#include "IndicatorStructres.h"

////////////////////////////////////////////////////
/////////// Base Indicator class Functions//////////
/////////// Author: Fahed.b				  //////////
////////////////////////////////////////////////////

void BaseIndicator::SetValues(IndicatorValues curr, IndicatorValues prev)
{
	m_curr_BarValues = curr;
}

BaseIndicator::BaseIndicator(IndicatorValues curr) {
	m_curr_BarValues = curr;
}


////////////////////////////////////////////////////
/////////// END OF Base Indicator class Functions////
////////////////////////////////////////////////////



////////////////////////////////////////////////////
// Gravity Values Structer class Functions//////////
///			Author: Fahed.b				  //////////
////////////////////////////////////////////////////

void Gravity::CalcGravityMembers(double open, double close)
{
	calcGravityLevel();
	setGravityType(open, close);
}

void Gravity::setGravityType(double close, double open)
{
	GravityType newType;
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();

	if (m_GravityStren < GRAVITY_MIN_STRONG_LEVEL)
	{
		newType = (close > open) ? GravityType::Random_Asc : GravityType::Random_Desc;
	}
	else if (m_UpperTicks > m_lowerTicks)
	{
		newType = (close > open) ? GravityType::High_Asc : GravityType::High_Desc;
	}
	else
	{
		newType = (close > open) ? GravityType::Low_Asc : GravityType::Low_Desc;
	}

	m_type = newType;

}

void Gravity::calcGravityLevel()
{
	int strongSide;
	if (m_UpperTicks > m_lowerTicks)
		strongSide = m_UpperTicks;
	else
		strongSide = -1 * m_lowerTicks;

	m_GravityStren = ((double)strongSide / (double)m_TotalTicks);
	m_GravityStren *= 100.0;
}

////////////////////////////////////////////////////
/////////// END OF Gravity Values Structer class Functions////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// Gravity Indricator class Functions	  //////////
///			Author: Fahed.b				  //////////
////////////////////////////////////////////////////

Gravity_Ind::Gravity_Ind(double open, double close, Gravity currGravity)
{
	m_value_M1.m_TotalTicks = currGravity.m_TotalTicks;
	m_value_M1.m_UpperTicks = currGravity.m_UpperTicks;
	m_value_M1.m_lowerTicks = currGravity.m_lowerTicks;
	m_value_M1.CalcGravityMembers(open, close);
	Assign_Invalid_Data();

}
////////////////////////////////////////////////////
/////////// END OF Gravity Indricator class Functions////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// IndicatorScore Struct Functions		  //////////
///			Author: Fahed.b				  //////////
////////////////////////////////////////////////////

IndicatorScore::IndicatorScore() {
	M1 = 0;
	M5 = 0;
	M15 = 0;
	M30 = 0;
	H1 = 0;
	H4 = 0;
	total = 0;
}

// Calculate the total score

void IndicatorScore::calcTotal() {
	total = M1 + M5 + M15 + M30 + H1 + H4;
}

IndicatorScore IndicatorScore::operator+(const IndicatorScore& other) const {
	IndicatorScore result;
	result.M1 = M1 + other.M1;
	result.M5 = M5 + other.M5;
	result.M15 = M15 + other.M15;
	result.M30 = M30 + other.M30;
	result.H1 = H1 + other.H1;
	result.H4 = H4 + other.H4;
	result.calcTotal(); // Recalculate the total
	return result;
}
IndicatorScore& IndicatorScore::operator+=(const IndicatorScore& other) {
	M1 += other.M1;
	M5 += other.M5;
	M15 += other.M15;
	M30 += other.M30;
	H1 += other.H1;
	H4 += other.H4;
	calcTotal();
	return *this;
}


////////////////////////////////////////////////////
/////////// END OF IndicatorScore Struct class Functions////
////////////////////////////////////////////////////


////////////////////////////////////////////////////
// ScoreResult Struct Functions		  //////////
///			Author: Fahed.b				  //////////
////////////////////////////////////////////////////

inline ScoresResult::ScoresResult()
{
	basicScore = 0;
	gravityScore = 0;
}

void ScoresResult::calcTotalResult() {
	rsiScore.calcTotal();
	emaScore.calcTotal();
	zlsmaScore.calcTotal();

	totalScore = combineScores();
	totalScore.calcTotal();
}

ScoresResult& ScoresResult::operator+=(const ScoresResult& other) {
	basicScore += other.basicScore;
	gravityScore += other.gravityScore;
	rsiScore += other.rsiScore;
	emaScore += other.emaScore;
	zlsmaScore += other.zlsmaScore;
	totalScore = combineScores();
	totalScore.calcTotal();

	return *this;
}

ScoresResult ScoresResult::operator+(const ScoresResult& other) const {
	ScoresResult result;

	// Add scalar scores
	result.basicScore = basicScore + other.basicScore;
	result.gravityScore = gravityScore + other.gravityScore;

	// Add IndicatorScore fields
	result.rsiScore = rsiScore + other.rsiScore;
	result.emaScore = emaScore + other.emaScore;
	result.zlsmaScore = zlsmaScore + other.zlsmaScore;
	// Combine total scores (no need to recalculate; just sum them)
	result.calcTotalResult();

	return result;
}

// Helper function to combine scores into totalScore

IndicatorScore ScoresResult::combineScores() const {
	IndicatorScore combined;
	combined.M1 = basicScore + gravityScore + rsiScore.M1 + emaScore.M1 + zlsmaScore.M1;
	combined.M5 = rsiScore.M5 + emaScore.M5 + zlsmaScore.M5;
	combined.M15 = rsiScore.M15 + emaScore.M15 + zlsmaScore.M15;
	combined.M30 = rsiScore.M30 + emaScore.M30 + zlsmaScore.M30;
	combined.H1 = rsiScore.H1 + emaScore.H1 + zlsmaScore.H1;
	combined.H4 = rsiScore.H4 + emaScore.H4 + zlsmaScore.H4;

	return combined;
}

////////////////////////////////////////////////////
/////////// END OF ScoreResult Struct class Functions////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// IndicatorMap Class Functions		  //////////
///			Author: Fahed.b				  //////////
////////////////////////////////////////////////////


IndicatorMap::IndicatorMap()
{
	m_map.insert(std::make_pair(std::make_pair(IndicatorType::EMA, IndicatorLen::L30), nullptr));
	m_map.insert(std::make_pair(std::make_pair(IndicatorType::EMA, IndicatorLen::L25), nullptr));
	m_map.insert(std::make_pair(std::make_pair(IndicatorType::EMA, IndicatorLen::L20), nullptr));
	m_map.insert(std::make_pair(std::make_pair(IndicatorType::EMA, IndicatorLen::L15), nullptr));
	m_map.insert(std::make_pair(std::make_pair(IndicatorType::EMA, IndicatorLen::L10), nullptr));

	m_map.insert(std::make_pair(std::make_pair(IndicatorType::RSI, IndicatorLen::L30), nullptr));
	m_map.insert(std::make_pair(std::make_pair(IndicatorType::RSI, IndicatorLen::L25), nullptr));
	m_map.insert(std::make_pair(std::make_pair(IndicatorType::RSI, IndicatorLen::L20), nullptr));
	m_map.insert(std::make_pair(std::make_pair(IndicatorType::RSI, IndicatorLen::L15), nullptr));
	m_map.insert(std::make_pair(std::make_pair(IndicatorType::RSI, IndicatorLen::L10), nullptr));

	m_map.insert(std::make_pair(std::make_pair(IndicatorType::ZLSMA, IndicatorLen::L30), nullptr));
	m_map.insert(std::make_pair(std::make_pair(IndicatorType::ZLSMA, IndicatorLen::L25), nullptr));
	m_map.insert(std::make_pair(std::make_pair(IndicatorType::ZLSMA, IndicatorLen::L20), nullptr));
	m_map.insert(std::make_pair(std::make_pair(IndicatorType::ZLSMA, IndicatorLen::L15), nullptr));
	m_map.insert(std::make_pair(std::make_pair(IndicatorType::ZLSMA, IndicatorLen::L10), nullptr));
}

void IndicatorMap::Set_Indicator(IndicatorType type, IndicatorLen len, IndicatorValues val,bool forceInsert /*false*/)
{
	std::pair<IndicatorType, IndicatorLen> pr = std::make_pair(type, len);
	if (m_map.find(pr) != m_map.end()) {
		if (!m_map[pr] || forceInsert)
			m_map[pr] = Get_IndicatorObject(type, val);
	}
	m_map.insert(std::make_pair(pr, Get_IndicatorObject(type, val)));

}

// Function to create a new Indicator object

BaseIndicator* IndicatorMap::Get_Indicator(IndicatorType type, IndicatorLen len)
{
	auto pr = std::make_pair(type, len);
	auto it = m_map.find(pr);

	return (it != m_map.end()) ? it->second : nullptr;
}

void IndicatorMap::Set_IndValue(BaseIndicator* ind, BarPeriod barP, double val)
{
	switch (barP)
	{
	case BarPeriod::M1:
		ind->Get_IndicatorValues_Pointer()->M1 = val;
		break;
	case BarPeriod::M5:
		ind->Get_IndicatorValues_Pointer()->M5 = val;
		break;
	case BarPeriod::M15:
		ind->Get_IndicatorValues_Pointer()->M15 = val;
		break;
	case BarPeriod::M30:
		ind->Get_IndicatorValues_Pointer()->M30 = val;
		break;
	case BarPeriod::H1:
		ind->Get_IndicatorValues_Pointer()->H1 = val;
		break;
	case BarPeriod::H4:
		ind->Get_IndicatorValues_Pointer()->H4 = val;
		break;
	case BarPeriod::NONE:
		break;
	default:
		break;
	}
}

void IndicatorMap::Set_IndSlope(BaseIndicator* ind, BarPeriod barP, double val)
{
	auto* values = ind->Get_IndicatorValues_Pointer();
	switch (barP)
	{
	case BarPeriod::M1:
		values->m_Slope_M1 = val;
		break;
	case BarPeriod::M5:
		values->m_Slope_M5 = val;
		break;
	case BarPeriod::M15:
		values->m_Slope_M15 = val;
		break;
	case BarPeriod::M30:
		values->m_Slope_M30 = val;
		break;
	case BarPeriod::H1:
		values->m_Slope_H1 = val;
		break;
	case BarPeriod::H4:
		values->m_Slope_H4 = val;
		break;
	default:
		break;
	}
}

void IndicatorMap::Validate_Indicator(IndicatorType type, IndicatorLen len)
{
	auto pr = std::make_pair(type, len);
	auto it = m_map.find(pr);

	if (it != m_map.end()) {
		if (!it->second) {
			it->second = Get_IndicatorObject(type, IndicatorValues());
		}
	}
	else {
		m_map.insert({ pr, Get_IndicatorObject(type, IndicatorValues()) });
	}
}
BaseIndicator* IndicatorMap::Get_IndicatorObject(IndicatorType type, IndicatorValues val)
{
	switch (type)
	{
	case IndicatorType::EMA:
		return new EMA_Ind(val);
	case IndicatorType::RSI:
		return new RSI_Ind(val);
	case IndicatorType::ZLSMA:
		return new ZLSMA_Ind(val);
	default:
		return nullptr;
	}
}

////////////////////////////////////////////////////
/////////// END OF IndicatorMap class Functions////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// IndicatorValues Struct Functions		  //////////
///			Author: Fahed.b				  //////////
////////////////////////////////////////////////////

void IndicatorValues::Set_SingleSlope(BarPeriod barP, double slope)
{
	switch (barP)
	{
	case BarPeriod::M1:
		m_Slope_M1 = slope;
		break;
	case BarPeriod::M5:
		m_Slope_M5 = slope;
		break;
	case BarPeriod::M15:
		m_Slope_M15 = slope;
		break;
	case BarPeriod::M30:
		m_Slope_M30 = slope;
		break;
	case BarPeriod::H1:
		m_Slope_H1 = slope;
		break;
	case BarPeriod::H4:
		m_Slope_H4 = slope;
		break;
	case BarPeriod::NONE:
		break;
	default:
		break;
	}
}
void IndicatorValues::Set_SingleValue(BarPeriod barP, double val)
{
	switch (barP)
	{
	case BarPeriod::M1:
		M1 = val;
		break;
	case BarPeriod::M5:
		M5 = val;
		break;
	case BarPeriod::M15:
		M15 = val;
		break;
	case BarPeriod::M30:
		M30 = val;
		break;
	case BarPeriod::H1:
		H1 = val;
		break;
	case BarPeriod::H4:
		H4 = val;
		break;
	case BarPeriod::NONE:
		break;
	default:
		break;
	}
}