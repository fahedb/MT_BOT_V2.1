#include "IndicatorMap.h"
#include "Logger.h"

////////////////////////////////////////////////////
// IndicatorMap Class Functions		  //////////
///			Author: Fahed.b				  //////////
////////////////////////////////////////////////////
IndicatorTypeList m_glob_indTypes_IndMap;
IndicatorLenList m_glob_indLens_IndMap;
IndicatorMap::IndicatorMap(IndicatorStorage* indStorage, int changeNodeIndex)
{
	m_indStorage = indStorage;
	m_changeNodeIndex = changeNodeIndex;
	m_storageMode = true;
	int rows = m_glob_indTypes_IndMap.types.size();
	int cols = m_glob_indLens_IndMap.lengths.size();
	m_typesLen = rows;
	m_lensLen = cols;
	m_vec = new BaseIndicator * [rows * cols]();

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			m_vec[i * cols + j] = m_indStorage->Get_IndicatorPointer(
				static_cast<IndicatorType>(i),
				static_cast<IndicatorLen>(j),
				m_changeNodeIndex
			);
		}
	}
}

IndicatorMap::IndicatorMap()
{
	m_indStorage = nullptr;
	m_changeNodeIndex = -1;
	m_storageMode = false;

	int rows = m_glob_indTypes_IndMap.types.size();
	int cols = m_glob_indLens_IndMap.lengths.size();

	m_typesLen = rows;
	m_lensLen = cols;

	m_vec = new BaseIndicator * [rows * cols];
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			m_vec[i * cols + j] = Get_IndicatorObject(static_cast<IndicatorType>(i), static_cast<IndicatorLen>(j), IndicatorValues());
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

void IndicatorMap::Set_Indicator(IndicatorType type, IndicatorLen len, IndicatorValues val, bool forceInsert /*false*/)
{
	int typeIdx = static_cast<int>(type);
	int lenIdx = static_cast<int>(len);

	if (typeIdx < 0 || typeIdx >= m_typesLen || lenIdx < 0 || lenIdx >= m_lensLen)
		return;

	int idx = typeIdx * m_lensLen + lenIdx;

	if (m_storageMode)
	{
		if (m_vec[idx])
			m_vec[idx]->Get_IndicatorValues() = val;
		return;
	}

	if (m_vec[idx] && !forceInsert)
	{
		m_vec[idx]->Get_IndicatorValues() = val;
		return;
	}

	if (m_vec[idx])
		delete m_vec[idx];

	m_vec[idx] = Get_IndicatorObject(type, len, val);
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
	if (typeIdx < 0 || typeIdx >= m_typesLen || lenIdx < 0 || lenIdx >= m_lensLen)
	{
		Logger::getInstance().WriteLog("IndicatorMap::Get_Indicator : Invalid IndicatorType or IndicatorLen index.", false, true, LogType::ERR);
		return nullptr;
	}
	return m_vec[typeIdx * m_lensLen + lenIdx]; // Return the indicator from the vector
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

	if (typeIdx < 0 || typeIdx >= m_glob_indTypes_IndMap.types.size() || lenIdx < 0 || lenIdx >= m_glob_indLens_IndMap.lengths.size())
	{
		Logger::getInstance().WriteLog("IndicatorMap::Validate_Indicator : Invalid IndicatorType or IndicatorLen index.", false, true, LogType::ERR);
		return;
	}

	if (!m_vec[typeIdx * m_lensLen + lenIdx])
	{
		m_vec[typeIdx * m_lensLen + lenIdx] = Get_IndicatorObject(type, len, IndicatorValues());
	}
}

// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

BaseIndicator* IndicatorMap::Get_IndicatorObject(IndicatorType type,IndicatorLen len, IndicatorValues val)
{
	if(m_indStorage && m_changeNodeIndex >= 0) {
		BaseIndicator* indPtr = m_indStorage->Get_IndicatorPointer(type, len, m_changeNodeIndex);
		if (indPtr) {
			indPtr->Get_IndicatorValues() = val; // Set the values for the indicator
			return indPtr;
		} else {
			Logger::getInstance().WriteLog("IndicatorMap::Get_IndicatorObject : Failed to get indicator pointer from storage.", false, true, LogType::ERR);
		}
	}

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