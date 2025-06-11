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


////////////////////////////////////////////////////
/////////// END OF Gravity Values Structer class Functions////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// Gravity Indricator class Functions	  //////////
///			Author: Fahed.b				  //////////
////////////////////////////////////////////////////

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
}

void ScoresResult::calcTotalResult() {
	rsiScore.calcTotal();
	emaScore.calcTotal();
	zlsmaScore.calcTotal();
	volumeScore.calcTotal();
	totalScore = combineScores();
	totalScore.calcTotal();
}

ScoresResult& ScoresResult::operator+=(const ScoresResult& other) {

	rsiScore += other.rsiScore;
	emaScore += other.emaScore;
	zlsmaScore += other.zlsmaScore;
	volumeScore += other.volumeScore;
	totalScore = combineScores();
	totalScore.calcTotal();

	return *this;
}

ScoresResult ScoresResult::operator+(const ScoresResult& other) const {
	ScoresResult result;

	// Add IndicatorScore fields
	result.rsiScore = rsiScore + other.rsiScore;
	result.emaScore = emaScore + other.emaScore;
	result.zlsmaScore = zlsmaScore + other.zlsmaScore;
	result.volumeScore = volumeScore + other.volumeScore;
	// Combine total scores (no need to recalculate; just sum them)
	result.calcTotalResult();

	return result;
}

// Helper function to combine scores into totalScore

IndicatorScore ScoresResult::combineScores() const {
	IndicatorScore combined;
	combined.M1 =	rsiScore.M1		+ emaScore.M1	+ zlsmaScore.M1		+ volumeScore.M1;
	combined.M5 =	rsiScore.M5		+ emaScore.M5	+ zlsmaScore.M5		+ volumeScore.M5;
	combined.M15 =	rsiScore.M15	+ emaScore.M15	+ zlsmaScore.M15	+ volumeScore.M15;
	combined.M30 =	rsiScore.M30	+ emaScore.M30	+ zlsmaScore.M30	+ volumeScore.M30;
	combined.H1 =	rsiScore.H1		+ emaScore.H1	+ zlsmaScore.H1		+ volumeScore.H1;
	combined.H4 =	rsiScore.H4		+ emaScore.H4	+ zlsmaScore.H4		+ volumeScore.H4;

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
	m_vec.resize(INDICATOR_TYPES_VEC_LEN); // Reserve space for 5 elements
	
	for (int i = 0; i < INDICATOR_TYPES_VEC_LEN; i++)
	{
		m_vec.at(i).resize(INDICATOR_LEN_VEC_LEN); // Reserve space for 5 elements in each vector
		for (int j = 0; j < INDICATOR_LEN_VEC_LEN; j++)
		{
			m_vec.at(i).at(j) = nullptr; // Initialize each element to nullptr
		}
	}
}

void IndicatorMap::Set_Indicator(IndicatorType type, IndicatorLen len, IndicatorValues val,bool forceInsert /*false*/)
{
	int typeIdx = static_cast<int>(Indicators_Utils::Get_TypeIndex(type));
	int lenIdx = static_cast<int>(Indicators_Utils::Get_LenIndex(len));
	if (typeIdx < 0 || typeIdx >= INDICATOR_TYPES_VEC_LEN || lenIdx < 0 || lenIdx >= INDICATOR_LEN_VEC_LEN)
	{
		Logger::getInstance().WriteLog("IndicatorMap::Set_Indicator : Invalid IndicatorType or IndicatorLen index.", false, true, LogType::ERR);
		return;
	}
	m_vec.at(typeIdx).at(lenIdx) = Get_IndicatorObject(type, val); // Store the indicator in the vector
}

// Function to create a new Indicator object

BaseIndicator* IndicatorMap::Get_Indicator(IndicatorType type, IndicatorLen len)
{
	int typeIdx = static_cast<int>(Indicators_Utils::Get_TypeIndex(type));
	int lenIdx = static_cast<int>(Indicators_Utils::Get_LenIndex(len));
	if (typeIdx < 0 || typeIdx >= INDICATOR_TYPES_VEC_LEN || lenIdx < 0 || lenIdx >= INDICATOR_LEN_VEC_LEN)
	{
		Logger::getInstance().WriteLog("IndicatorMap::Get_Indicator : Invalid IndicatorType or IndicatorLen index.", false, true, LogType::ERR);
		return nullptr;
	}
	return m_vec.at(typeIdx).at(lenIdx); // Return the indicator from the vector
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
	int typeIdx = static_cast<int>(Indicators_Utils::Get_TypeIndex(type));
	int lenIdx = static_cast<int>(Indicators_Utils::Get_LenIndex(len));

	if (typeIdx < 0 || typeIdx >= INDICATOR_TYPES_VEC_LEN || lenIdx < 0 || lenIdx >= INDICATOR_LEN_VEC_LEN)
	{
		Logger::getInstance().WriteLog("IndicatorMap::Validate_Indicator : Invalid IndicatorType or IndicatorLen index.", false, true, LogType::ERR);
		return;
	}

	if (!m_vec.at(typeIdx).at(lenIdx))
	{
		m_vec.at(typeIdx).at(lenIdx) = Get_IndicatorObject(type, IndicatorValues());
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
	case IndicatorType::VOLUME:
		return new Volume_Ind(val);
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