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

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------


////////////////////////////////////////////////////
// IndicatorScore Struct Functions		  //////////
///			Author: Fahed.b				  //////////
////////////////////////////////////////////////////

// Just a constructor :)
IndicatorScore::IndicatorScore() {
	BarPeriodList barPeriods;
	for(const auto& period : barPeriods.periods) {
		scores.at(period) = 0.0; // Initialize each BarPeriod with a score of 0
	}
	total = 0;
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

void IndicatorScore::calcTotal() { // Calculate the total score
	total = 0; // Reset total before calculation
	for (const auto& pair : scores) {
		total += pair.second; // Sum up all the scores
	}
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

IndicatorScore IndicatorScore::operator+(const IndicatorScore& other) const { 
	IndicatorScore result;

	for(const auto& pair : scores) {
		BarPeriod period = pair.first;
		result.scores.at(period) = pair.second + other.scores.at(period); // Sum scores for each BarPeriod
	}
	result.calcTotal(); // Recalculate the total
	return result;
}
IndicatorScore& IndicatorScore::operator+=(const IndicatorScore& other) {
	for(const auto& pair : scores) {
		BarPeriod period = pair.first;
		scores.at(period) += other.scores.at(period); // Add scores for each BarPeriod
	}
	calcTotal(); // Recalculate the total
	return *this;
}
IndicatorScore IndicatorScore::operator-(const IndicatorScore& other) const
{
	IndicatorScore result;

	for (const auto& pair : scores) {
		BarPeriod period = pair.first;
		double res = pair.second - other.scores.at(period);
		result.scores.at(period) = res >= 0 ? res : 0; // Sum scores for each BarPeriod
	}
	result.calcTotal(); // Recalculate the total
	return result;
}
IndicatorScore IndicatorScore::operator/(const IndicatorScore& other) const
{
	IndicatorScore result;
	for (const auto& pair : scores) {
		if (other.scores.at(pair.first) != 0) { // Avoid division by zero
			result.scores[pair.first] = pair.second / other.scores.at(pair.first);
		}
		else {
			result.scores[pair.first] = 0.0; // Default to 0 if division by zero
		}
	}
	result.calcTotal(); // Update the total score after division
	return result;
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

IndicatorScore operator*(const IndicatorScore& score, double factor) // External function to multiply IndicatorScore by a factor
{
	IndicatorScore result;

	for (auto it = score.scores.begin(); it != score.scores.end(); ++it)
	{
		result.scores[it->first] = it->second * factor; // Scale each score (double) by the factor
	}
	result.calcTotal(); // Recalculate the total score after scaling

	return result;
}

IndicatorScore operator*(double factor, const IndicatorScore& score) // External function to multiply IndicatorScore by a factor
{
	return score * factor; // Just call the other operator
}

////////////////////////////////////////////////////
/////////// END OF IndicatorScore Struct class Functions////
////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------



////////////////////////////////////////////////////
// ScoreResult Struct Functions		  //////////
///			Author: Fahed.b				  //////////
////////////////////////////////////////////////////

ScoresResult::ScoresResult() // Just a constructor :)
{
	IndicatorTypeList indTypes;
	for (const auto& type : indTypes.types) {
		indicatorScores.at(type) = IndicatorScore(); // Initialize each IndicatorType with an empty IndicatorScore
	}
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

void ScoresResult::calcTotalResult() { // Calc total score of every object
	for(auto& pair : indicatorScores) {
		pair.second.calcTotal(); // Calculate total for each IndicatorScore
	}

	totalScore = combineScores();
	totalScore.calcTotal();
}

inline void ScoresResult::Reset() { // Clear the scores and total
	for (auto& pair : indicatorScores) {
		pair.second.Reset(); // Reset each IndicatorScore
	}
	totalScore.Reset(); // Reset the total score
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

ScoresResult& ScoresResult::operator+=(const ScoresResult& other) {

	IndicatorTypeList indTypes;
	for (const auto& type : indTypes.types) {
		indicatorScores.at(type) += other.indicatorScores.at(type); // Add scores for each IndicatorType
	}

	totalScore = combineScores();
	totalScore.calcTotal();

	return *this;
}
ScoresResult ScoresResult::operator-(const ScoresResult& other) const
{

	ScoresResult result;
	IndicatorTypeList indList;

	for (const auto& type : indList.types) { // Iterate through all IndicatorTypes
		result.indicatorScores.at(type) = indicatorScores.at(type) - other.indicatorScores.at(type); // Combine scores for each IndicatorType
	}
	result.calcTotalResult(); // Combine total scores (no need to recalculate; just sum them)

	return result;

}
ScoresResult ScoresResult::operator/(const ScoresResult& other) const
{
	ScoresResult result;
	for (const auto& pair : indicatorScores) {
		if (other.indicatorScores.at(pair.first).total != 0) { // Avoid division by zero
			result.indicatorScores[pair.first] = pair.second / other.indicatorScores.at(pair.first);
		}
		else {
			result.indicatorScores[pair.first].Reset(); // Reset to default if division by zero
		}
	}
	result.calcTotalResult(); // Update the total score after division
	return result;
}
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

ScoresResult ScoresResult::operator+(const ScoresResult& other) const {
	
	ScoresResult result;
	IndicatorTypeList indList;

	for(const auto& type : indList.types) { // Iterate through all IndicatorTypes
		result.indicatorScores.at(type) = indicatorScores.at(type) + other.indicatorScores.at(type); // Combine scores for each IndicatorType
	}
	result.calcTotalResult(); // Combine total scores (no need to recalculate; just sum them)

	return result;
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

IndicatorScore ScoresResult::combineScores() const { // function to combine scores into totalScore

	IndicatorScore combined;
	combined.Reset(); // Reset combined scores

	BarPeriodList barPeriods;

	for(const auto& period : barPeriods.periods) { // Iterate through all BarPeriods
		if (period == BarPeriod::NONE) continue; // Skip NONE period
		IndicatorTypeList indTypes;
		for(auto& type : indTypes.types) { // Iterate through all IndicatorTypes
			combined.scores.at(period) += indicatorScores.at(type).scores.at(period); // Sum scores for each BarPeriod and IndicatorType
		}
	}
	return combined;
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

ScoresResult operator*(const ScoresResult& score, double factor) // External function to multiply ScoresResult by a factor
{
	ScoresResult result;

	for (auto it = score.indicatorScores.begin(); it != score.indicatorScores.end(); ++it)
	{
		IndicatorScore scaledScore = it->second * factor; // ãåøù ùâí ìÎIndicatorScore éäéä operator*
		result.indicatorScores[it->first] = scaledScore;
	}
	result.calcTotalResult(); // Recalculate the total score after scaling
	return result;
}

ScoresResult operator*(double factor, const ScoresResult& score) // External function to multiply ScoresResult by a factor
{
	return score * factor;
}

/////////////////////////////////////////////////////////
/////////// END OF ScoreResult Struct class Functions////
/////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------


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

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

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

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// To Set indicator single value - by type, len and baePeriod
void IndicatorMap::Set_Ind_SingleValue(IndicatorType type, IndicatorLen len, BarPeriod barP, double val)
{
	Validate_Indicator(type, len); // Create the Indicator obj if it's missing - by type and len
	BaseIndicator* ind = Get_Indicator(type, len); // Getting the indicator ptr - by type and len
	Set_IndValue(ind, barP, val); // Set a single indicator value - by barPeriod
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// To Set indicator single value - by type, len and baePeriod
void IndicatorMap::Set_Ind_SingleSlope(IndicatorType type, IndicatorLen len, BarPeriod barP, double val) // To Set indicator single slope - by type, len and baePeriod
{
	Validate_Indicator(type, len); // Create the Indicator obj if it's missing - by type and len
	BaseIndicator* ind = Get_Indicator(type, len); // Getting the indicator ptr - by type and len
	Set_IndSlope(ind, barP, val); // Set a single indicator value - by barPeriod
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

BaseIndicator* IndicatorMap::Get_Indicator(IndicatorType type, IndicatorLen len) // Function to Get Indicaor ptr
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

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

void IndicatorMap::Set_IndValue(BaseIndicator* ind, BarPeriod barP, double val) // Function to create a new Indicator object
{
	if (!ind) {
		Logger::getInstance().WriteLog("IndicatorMap::Set_IndValue : BaseIndicator pointer is null.", false, true, LogType::ERR);
		return;
	}
	ind->Get_IndicatorValues_Pointer()->Set_SingleValue(barP, val); // Set the slope value for the specific BarPeriod
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

void IndicatorMap::Set_IndSlope(BaseIndicator* ind, BarPeriod barP, double val)
{
	if (!ind) {
		Logger::getInstance().WriteLog("IndicatorMap::Set_IndValue : BaseIndicator pointer is null.", false, true, LogType::ERR);
		return;
	}
	ind->Get_IndicatorValues_Pointer()->Set_Value(IndicatorValueType::Slope, barP, val); // Set the slope value for the specific BarPeriod
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

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

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

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

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

////////////////////////////////////////////////////
/////////// END OF IndicatorMap class Functions////
////////////////////////////////////////////////////

////////////////////////////////////////////////////
// IndicatorValues Struct Functions		  //////////
///			Author: Fahed.b				  //////////
////////////////////////////////////////////////////


