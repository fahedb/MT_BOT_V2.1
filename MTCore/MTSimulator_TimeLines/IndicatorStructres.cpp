#include "IndicatorStructres.h"
#include "LOGGER.H"

////////////////////////////////////////////////////
/////////// Base Indicator class Functions//////////
/////////// Author: Fahed.b				  //////////
////////////////////////////////////////////////////
const IndicatorTypeList m_glob_indTypes;
const IndicatorLenList m_glob_indLens;
const BarPeriodList m_glob_barPeriods;

const int m_IndTypesLen = m_glob_indTypes.types.size();
const int m_indLensLen = m_glob_indLens.lengths.size();

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
	total = 0;
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

void IndicatorScore::calcTotal() { // Calculate the total score
	total = 0; // Reset total before calculation
	const int mySize = m_size;
	for (int i = 0; i < m_size; i++) {
		total += m_scores[i]; // Sum up all the scores
	}
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

IndicatorScore IndicatorScore::operator+(const IndicatorScore& other) const { 
	IndicatorScore result;
	const int mySize = m_size;
	for (int i = 0; i < mySize; i++) {
		result.m_scores[i] = m_scores[i] + other.m_scores[i]; // Sum scores for each BarPeriod
	}
	result.calcTotal(); // Recalculate the total
	return result;
}
IndicatorScore& IndicatorScore::operator+=(const IndicatorScore& other) {
	
	const size_t mySize = m_size;
	
	for (size_t i = 0; i < mySize; ++i)
		m_scores[i] += other.m_scores[i];

	calcTotal(); // Recalculate the total
	return *this;
}
IndicatorScore IndicatorScore::operator-(const IndicatorScore& other) const
{
	IndicatorScore result;

	const int myScore = getSize();

	for (int i = 0; i < myScore; i++) {
		double res = m_scores[i] - other.m_scores[i];
		result.m_scores[i] = res >= 0 ? res : 0; // Sum scores for each BarPeriod
	}
	result.calcTotal(); // Recalculate the total
	return result;
}
IndicatorScore IndicatorScore::operator/(const IndicatorScore& other) const
{
	IndicatorScore result;
	
	const int mySize = getSize();

	for (int i = 0; i < mySize; i++) {

		if (other.m_scores[i] != 0) { // Avoid division by zero
			result.m_scores[i] = m_scores[i] / other.m_scores[i];
		}
		else {
			result.m_scores[i] = 0.0; // Default to 0 if division by zero
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
	const int mySize = score.getSize();

	for (int idx = 0; idx < mySize; idx++)
	{
		result.m_scores[idx] = score.m_scores[idx] * factor; // Scale each score (double) by the factor
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
	indicatorScores.assign(m_IndTypesLen, IndicatorScore()); // Initialize the vector with empty IndicatorScores
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

void ScoresResult::calcTotalResult() { // Calc total score of every object
	for(auto& indScore : indicatorScores) {
		indScore.calcTotal(); // Calculate total for each IndicatorScore
	}

	totalScore = combineScores();
	totalScore.calcTotal();
}

void ScoresResult::Reset() { // Clear the scores and total
	for (auto& pair : indicatorScores) {
		pair.Reset(); // Reset each IndicatorScore
	}
	totalScore.Reset(); // Reset the total score
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

ScoresResult& ScoresResult::operator+=(const ScoresResult& other) {


	for (const auto type : m_glob_indTypes.types) {
		indicatorScores[int(type)] += other.indicatorScores[int(type)]; // Add scores for each IndicatorType
	}

	totalScore = combineScores();
	totalScore.calcTotal();

	return *this;
}
ScoresResult ScoresResult::operator-(const ScoresResult& other) const
{

	ScoresResult result;

	for (const auto& type : m_glob_indTypes.types) { // Iterate through all IndicatorTypes
		result.indicatorScores[int(type)] = indicatorScores[int(type)] - other.indicatorScores[int(type)]; // Combine scores for each IndicatorType
	}
	result.calcTotalResult(); // Combine total scores (no need to recalculate; just sum them)

	return result;

}
ScoresResult ScoresResult::operator/(const ScoresResult& other) const
{
	ScoresResult result;
	int idx = 0;
	for (const auto res : indicatorScores) {
		if (other.indicatorScores[idx].total != 0) { // Avoid division by zero
			result.indicatorScores[idx] = res / other.indicatorScores[idx];
		}
		else {
			result.indicatorScores[idx].Reset(); // Reset to default if division by zero
		}
		idx++;
	}
	result.calcTotalResult(); // Update the total score after division
	return result;
}
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

ScoresResult ScoresResult::operator+(const ScoresResult& other) const {
	
	ScoresResult result;
	
	for(int i=0 ; i < other.indicatorScores.size(); i++)
	{
		result.indicatorScores[i] = indicatorScores[i] + other.indicatorScores[i]; // Combine scores for each IndicatorType
	}
	result.calcTotalResult(); // Combine total scores (no need to recalculate; just sum them)

	return result;
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

IndicatorScore ScoresResult::combineScores() const { // function to combine scores into totalScore

	IndicatorScore combined;
	combined.Reset(); // Reset combined scores
	const int barPeriodsSize = combined.getSize();
	const int indTypesSize = m_glob_indTypes.types.size();

	for (int p = 0; p < barPeriodsSize; p++) { // Iterate through all BarPeriods
		for (int t = 0; t < indTypesSize; t++) { // Iterate through all IndicatorTypes
			combined.m_scores[p] += indicatorScores[t].m_scores[p]; // Sum scores for each BarPeriod and IndicatorType
		}
	}
	return combined;
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

ScoresResult operator*(const ScoresResult& score, double factor) // External function to multiply ScoresResult by a factor
{
	ScoresResult result;

	for (int idx = 0; idx < score.indicatorScores.size(); idx++)
	{
		IndicatorScore scaledScore = score.indicatorScores[idx] * factor; // ãåøù ùâí ìÎIndicatorScore éäéä operator*
		result.indicatorScores[idx] = scaledScore;
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
	m_vec.resize(m_IndTypesLen); // Reserve space for 5 elements
	
	for (int i = 0; i < m_IndTypesLen; i++)
	{
		m_vec.at(i).resize(m_indLensLen); // Reserve space for 5 elements in each vector
		for (int j = 0; j < m_indLensLen; j++)
		{
			m_vec.at(i).at(j) = nullptr; // Initialize each element to nullptr
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

void IndicatorMap::Set_Indicator(IndicatorType type, IndicatorLen len, IndicatorValues val,bool forceInsert /*false*/)
{
	int typeIdx = static_cast<int>(type);
	int lenIdx = static_cast<int>(len);
	if (typeIdx < 0 || typeIdx >= m_indLensLen || lenIdx < 0 || lenIdx >= m_indLensLen)
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
	int typeIdx = static_cast<int>(type);
	int lenIdx = static_cast<int>((len));
	if (typeIdx < 0 || typeIdx >= m_IndTypesLen || lenIdx < 0 || lenIdx >= m_indLensLen)
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
	int typeIdx = static_cast<int>(type);
	int lenIdx = static_cast<int>(len);

	if (typeIdx < 0 || typeIdx >= m_glob_indTypes.types.size() || lenIdx < 0 || lenIdx >= m_glob_indLens.lengths.size())
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

void IndicatorValues::Set_Value(IndicatorValueType valueType, BarPeriod barP, double val) // General function to set value or slope based on IndicatorValueType
{
	if (valueType == IndicatorValueType::Value)
		Set_SingleValue(barP, val);
	else if (valueType == IndicatorValueType::Slope)
		Set_SingleSlope(barP, val);
	else
		Logger::getInstance().WriteLog("IndicatorValues::Set_Value : Invalid IndicatorValueType.", false, true, LogType::ERR);
}
