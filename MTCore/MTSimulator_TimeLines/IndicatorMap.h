#pragma once
#include "INDICATORSTRUCTRES.H"
#include "IndicatorsStorage.h"

class IndicatorMap
{
private:
	BaseIndicator** m_vec;
	IndicatorStorage* m_indStorage;
	int m_changeNodeIndex;
	bool m_storageMode; // Flag to indicate if using storage mode

	int m_typesLen;
	int m_lensLen;
	BaseIndicator* Get_IndicatorObject(IndicatorType type, IndicatorLen len, IndicatorValues val); // Getting a pointer of an allocated indicator Object
public:
	IndicatorMap(); // Contructor
	IndicatorMap(IndicatorStorage* indStorage, int changeNodeIndex); // Contructor
	~IndicatorMap()
	{
		if (!m_storageMode) // Only delete the indicators if we are not in storage mode, since in storage mode the indicators are managed by the IndicatorStorage class
		{
			for (int i = 0; i < m_typesLen * m_lensLen; i++)
				delete m_vec[i];
		}

		delete[] m_vec; // Delete the array of pointers
		m_vec = nullptr;
	}
	void Set_Indicator(IndicatorType, IndicatorLen, IndicatorValues, bool forceInsert = false); // Function to create a new Indicator object
	void Set_Ind_SingleValue(IndicatorType type, IndicatorLen len, BarPeriod barP, double val); // To Set indicator single value - by type, len and baePeriod
	void Set_Ind_SingleSlope(IndicatorType type, IndicatorLen len, BarPeriod barP, double val); // To Set indicator single slope - by type, len and barPeriod
	BaseIndicator* Get_Indicator(IndicatorType type, IndicatorLen len); // Get Indicator ptr
private:
	void Set_IndValue(BaseIndicator*, BarPeriod, double);
	void Set_IndSlope(BaseIndicator*, BarPeriod, double);
	void Validate_Indicator(IndicatorType type, IndicatorLen len); // A function to check if there is an object for the relevant indicator, if not it will creat one and insert it in the map
};