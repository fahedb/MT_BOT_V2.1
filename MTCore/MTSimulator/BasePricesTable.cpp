#include "BasePricesTable.h"
#include "Logger.h"

IndicatorValues InitIndicatorValues(std::string M1, std::string M5, std::string M15, std::string M30, std::string H1, std::string H4);
IndicatorValues InitIndicatorValues(std::string M1, std::string M5, std::string M15, std::string M30, std::string H1, std::string H4, std::string M1_s, std::string M5_s, std::string M15_s, std::string M30_s, std::string H1_s, std::string H4_s);
void BasePricesTable::ClearAll()
{
	m_FPriceDateTime = "";
	m_FOpenPrice = 0;
	m_FClosePrice = 0;
	m_FActionSubmitted = 0;

	m_IPriceDateTime = "";
	m_IBuyPrice = 0;
	m_ISellPrice = 0;
	m_IActionSubmitted = 0;
	SortResults = false;
	RemoveSelection();
}

void BasePricesTable::ClearQuery()
{
	m_IPriceDateTime = "";
	m_IBuyPrice = 0;
	m_ISellPrice = 0;
	m_IActionSubmitted = 0;
	SortResults = false;
}

void BasePricesTable::ClearInsert()
{
	m_FPriceDateTime = "";
	m_FOpenPrice= 0;
	m_FClosePrice = 0;
	m_FActionSubmitted = 0;
}


void BasePricesTable::addFilter(const std::string& filter, std::string& query)
{
	int idx = query.find("WHERE");
	if (idx >= 0)
		query += " AND ";
	else
		query += " WHERE ";
	query += (filter);
}
void BasePricesTable::addSort(std::string& query)
{
	if (m_PriceDateTime_AscDescSort)
		query += " order by PriceDateTime asc";
	else
		query += " order by PriceDateTime desc";
}
bool BasePricesTable::SubmitQuery()
{
	std::string querySQLCommand = "SELECT * FROM " + m_tableName + " (NOLOCK) ";

	if (m_IPriceDateTime.length())
		addFilter(m_IPriceDateTime, querySQLCommand);

	if (m_IBuyPrice)
		addFilter(std::to_string(m_IBuyPrice), querySQLCommand);

	if (m_ISellPrice)
		addFilter(std::to_string(m_IBuyPrice), querySQLCommand);

	if (m_IActionSubmitted)
		addFilter(std::to_string(m_IActionSubmitted), querySQLCommand);
	if (SortResults)
		addSort(querySQLCommand);
	SelectFromDB(querySQLCommand, GetColsNum());
	return true;
}

bool BasePricesTable::SubmitInsert()
{
	std::string insertSQLCommand = "INSERT INTO " + m_tableName + " (PriceDateTime, OpenPrice,ClosePrice,ActionSubmitted) VALUES('" + m_FPriceDateTime + "'," + std::to_string(m_FOpenPrice) + "," + std::to_string(m_FClosePrice) + "," + std::to_string(m_FActionSubmitted) + ")";
	bool result = InsertIntoDB(insertSQLCommand);
	ClearInsert();

	return result;
}

SQLSelectionNode* BasePricesTable::GetPrevEnt()
{
	if (index == 0)
		return NULL;
	int currIdx = index - 1;
	return GetSelected()->at(currIdx);

}

FileSelectionNode* BasePricesTable::GetPrevFileEnt()
{
	if (index == 0)
		return NULL;
	int currIdx = index - 1;
	return GetSelectedFromFile()->at(currIdx);

}

SQLSelectionNode* BasePricesTable::GetNextEnt()
{
	if (GetSelected()->size() <= index)
		return NULL;
	int currIdx = index;
	index++;
	return GetSelected()->at(currIdx);

}

FileSelectionNode* BasePricesTable::GetNextFileEnt()
{
	if (GetSelectedFromFile()->size() <= index)
		return NULL;
	int currIdx = index;

	index++;
	return GetSelectedFromFile()->at(currIdx);
}

ChangeNode* BasePricesTable::GetNextChangeNode_MultiThreaded(int threadIdx)
{
	JsonSelectionNode* j = GetNextJsonEntity_MultiThreaded(threadIdx);
	if (j)
	{
		return BuildChangeNode(j);
	}
	return nullptr;
}

ChangeNode* BasePricesTable::GetNextChangeNode(ChangeNode* prev_chN)
{
	JsonSelectionNode* j = GetNextJsonEntity();
	if (j)
	{
		ChangeNode* chN = BuildChangeNode(j);
		//delete j;
		j = nullptr;
		return chN;
	}

	if (GetSelectedFromFile()->size() > index)
	{
		return BuildChangeNode(GetNextFileEnt(),prev_chN);
	}

	if (GetSelected()->size() > index)
	{
		ChangeNode* chN2= BuildChangeNode(GetNextEnt());
		return chN2;
	}
	return nullptr;
		
	
}

ChangeNode* BasePricesTable::BuildChangeNode(JsonSelectionNode* selectionJ)
{
	Logger& logger = Logger::getInstance();
	ChangeNode* chN = new ChangeNode();
	std::map<std::string, std::string> mapy;
	Http_Utils::ExtractAllValues(*(selectionJ->Get_Json()), mapy);
	bool timePointSwitch = false;
	for (auto it = mapy.begin(); it != mapy.end(); ++it) {
		
		std::vector<std::string> CurrKey = Http_Utils::SplitKey(it->first);
		std::string cleanedValue = Http_Utils::ExtractValuesFrom(it->second);
		
		if (CurrKey.size() < 2)
			continue;

		if (CurrKey[1] == HTTP_BODY_MEMBER_IND_FATHER)
		{
			if (CurrKey.size() < 6)
			{
				logger.WriteLog("Error in Json Structure !!!\nExit building Node !!!", false, true);
				logger.WriteLog("Error in Json Structure !!!\nExit building Node !!!", false, true,LogType::SPECIAL_LOG);
				return nullptr;
			}
			auto indT = Http_Utils::Get_Enum_IndicatorType(CurrKey[2]);
			auto indL = Http_Utils::Get_Enum_IdicatorLen(CurrKey[3]);
			auto indVT = Http_Utils::Get_Enum_IndicatorValueType(CurrKey[4]);
			auto barP =  Http_Utils::Get_Enum_BarPeriod(CurrKey[5]);
			if (indVT == IndicatorValueType::Slope)
				chN->Get_Indicators_Map()->Set_Ind_SingleSlope(indT, indL, barP, std::stod(cleanedValue));
			else if (indVT == IndicatorValueType::Value)
				chN->Get_Indicators_Map()->Set_Ind_SingleValue(indT, indL, barP, std::stod(cleanedValue));
		}
		else if (CurrKey[1] == HTTP_BODY_MEMBER_Symbol)
		{
			chN->symbol = cleanedValue;
		}
		else {
			chN->Set_Entity(CurrKey[1], std::stod(cleanedValue));
			if (!timePointSwitch)
			{
				chN->point_DateTime = CurrKey.at(0);
				timePointSwitch = true;
			}
		}
		
	}
	chN->activePoint = true;
	chN->Pip_Change = ProcessData::roundToDecimalPlaces(chN->Get_Entity(HTTP_BODY_MEMBER_ClosePrice), (int)GetPointPlace()) 
					- ProcessData::roundToDecimalPlaces(chN->Get_Entity(HTTP_BODY_MEMBER_OpenPrice), (int)GetPointPlace());
	return chN;
}



ChangeNode* BasePricesTable::BuildChangeNode(SQLSelectionNode* row, ChangeNode* prev_CHn)
{
	if (!row)
	{
		ChangeNode* chN = new ChangeNode();
		std::string pointName;
		ProcessData::ConvertSQLCHARToString(row->GetRow()->at(0).data(), pointName);

		chN->point_DateTime = pointName;
		chN->activePoint = false;
		chN->Pip_Change = 0;
		return chN;
	}
	ChangeNode* chN = new ChangeNode();
	std::string pointName;
	ProcessData::ConvertSQLCHARToString(row->GetRow()->at(0).data(), pointName);

	chN->point_DateTime = pointName;

	chN->activePoint = true;
	std::string OpenPStr;
	ProcessData::ConvertSQLCHARToString(row->GetRow()->at(1).data(), OpenPStr);

	std::string CloseStr = row->GetColumn(1);
	double doubleOpenPrice = std::stod(row->GetColumn(1));

	double doubleClosePrice = std::stod(row->GetColumn(2));


	int OpenPip = ProcessData::roundToDecimalPlaces(std::stod(OpenPStr), (int)GetPointPlace());
	int ClosePip = ProcessData::roundToDecimalPlaces(std::stod(CloseStr), (int)GetPointPlace());

	chN->Set_Entity(HTTP_BODY_MEMBER_OpenPrice, doubleOpenPrice);
	chN->Set_Entity(HTTP_BODY_MEMBER_ClosePrice, doubleClosePrice);

	chN->Set_Entity(HTTP_BODY_MEMBER_HighPrice, std::stod(row->GetColumn(3)));
	chN->Set_Entity(HTTP_BODY_MEMBER_LowPrice, std::stod(row->GetColumn(4)));
	chN->Pip_Change = ClosePip - OpenPip;

	// setting RSI indicator

	IndicatorValues currEMA = InitIndicatorValues(row->GetColumn(5), row->GetColumn(6), row->GetColumn(7), row->GetColumn(8), row->GetColumn(9), row->GetColumn(10), row->GetColumn(11), row->GetColumn(12), row->GetColumn(13), row->GetColumn(14), row->GetColumn(15), row->GetColumn(16));
	//chN->Set_EMA(currEMA);
	chN->Set_Indicator(IndicatorType::EMA, IndicatorLen::L30, currEMA);
	IndicatorValues currZLSMA = InitIndicatorValues(row->GetColumn(17), row->GetColumn(18), row->GetColumn(19), row->GetColumn(20), row->GetColumn(21), row->GetColumn(22), row->GetColumn(23), row->GetColumn(24), row->GetColumn(25), row->GetColumn(26), row->GetColumn(27), row->GetColumn(28));
	//chN->Set_ZLSMA(currZLSMA);
	chN->Set_Indicator(IndicatorType::ZLSMA, IndicatorLen::L30, currZLSMA);
	// Setting EMA indicator
	IndicatorValues currRSI = InitIndicatorValues(row->GetColumn(29), row->GetColumn(30), row->GetColumn(31), row->GetColumn(32), row->GetColumn(33), row->GetColumn(34), row->GetColumn(35), row->GetColumn(36), row->GetColumn(37), row->GetColumn(38), row->GetColumn(39), row->GetColumn(40)); 
	//chN->Set_RSI(currRSI);
	chN->Set_Indicator(IndicatorType::RSI, IndicatorLen::L30, currRSI);

	Gravity grav;
	grav.m_TotalTicks = std::stoi(row->GetColumn(41));
	grav.m_UpperTicks = std::stoi(row->GetColumn(42));
	grav.m_lowerTicks = std::stoi(row->GetColumn(43));

	chN->SetGravity(grav);
	if (chN->Pip_Change >= 0)
		chN->Increase_Decrease = true;
	else
		chN->Increase_Decrease = false;

	return chN;
}


ChangeNode* BasePricesTable::BuildChangeNode(FileSelectionNode* row, ChangeNode* prev_chN )
{
	if (!row)
	{
		ChangeNode* chN = new ChangeNode();
		std::string pointName = "99999";
		chN->point_DateTime = pointName;
		chN->activePoint = false;
		chN->Pip_Change = 0;
		return chN;
	}
	ChangeNode* chN = new ChangeNode();
	chN->point_DateTime = row->GetColumn(0);;

	chN->activePoint = true;
	

	int OpenPip = ProcessData::roundToDecimalPlaces(std::stod(row->GetColumn(1)), (int)GetPointPlace());
	int ClosePip = ProcessData::roundToDecimalPlaces(std::stod(row->GetColumn(2)), (int)GetPointPlace());

	chN->Set_Entity(HTTP_BODY_MEMBER_OpenPrice, std::stod(row->GetColumn(1)));
	chN->Set_Entity(HTTP_BODY_MEMBER_ClosePrice, std::stod(row->GetColumn(2)));

	chN->Set_Entity(HTTP_BODY_MEMBER_HighPrice, std::stod(row->GetColumn(3)));

	chN->Set_Entity(HTTP_BODY_MEMBER_LowPrice, std::stod(row->GetColumn(4)));

	chN->Pip_Change = ClosePip - OpenPip;

	// setting RSI indicator

	//IndicatorValues currEMA = InitIndicatorValues(row->GetColumn(5), row->GetColumn(6), row->GetColumn(7), row->GetColumn(8), row->GetColumn(9), row->GetColumn(10), row->GetColumn(11), row->GetColumn(12), row->GetColumn(13), row->GetColumn(14), row->GetColumn(15), row->GetColumn(16));
	//chN->Set_EMA(currEMA);

	// Setting EMA indicator
	//IndicatorValues currRSI = InitIndicatorValues(row->GetColumn(17), row->GetColumn(18), row->GetColumn(19), row->GetColumn(20), row->GetColumn(21), row->GetColumn(22), row->GetColumn(23), row->GetColumn(24), row->GetColumn(25), row->GetColumn(26), row->GetColumn(27), row->GetColumn(28));
	//chN->Set_RSI(currRSI);

	//IndicatorValues currZLSMA = InitIndicatorValues(row->GetColumn(29), row->GetColumn(30), row->GetColumn(31), row->GetColumn(32), row->GetColumn(33), row->GetColumn(34), row->GetColumn(35), row->GetColumn(36), row->GetColumn(37), row->GetColumn(38), row->GetColumn(39), row->GetColumn(40));
	//chN->Set_ZLSMA(currZLSMA);

	Gravity grav;
	grav.m_TotalTicks = std::stoi(row->GetColumn(41));
	grav.m_UpperTicks = std::stoi(row->GetColumn(42));
	grav.m_lowerTicks = std::stoi(row->GetColumn(43));

	chN->SetGravity(grav);
	if (chN->Pip_Change >= 0)
		chN->Increase_Decrease = true;
	else
		chN->Increase_Decrease = false;
	return chN;
}

IndicatorValues InitIndicatorValues(std::string M1, std::string M5, std::string M15, std::string M30, std::string H1, std::string H4) // OLD LOGIC !!!
{
	IndicatorValues ind;
	ind.H1 = 1;
	/*ind.M1 = std::stod(M1);
	ind.M5 = std::stod(M5);
	ind.M15 = std::stod(M15);
	ind.M30 = std::stod(M30);
	ind.H1 = std::stod(H1);
	ind.H4 = std::stod(H4);*/
	return ind;
}

IndicatorValues InitIndicatorValues(std::string M1, std::string M5, std::string M15, std::string M30, std::string H1, std::string H4, std::string M1_s, std::string M5_s, std::string M15_s, std::string M30_s, std::string H1_s, std::string H4_s)
{
	// OLD LOGIC !!!
	
	IndicatorValues ind;
	ind.H1 = 1;
	//ind.M1 = std::stod(M1);
	//ind.M5 = std::stod(M5);
	//ind.M15 = std::stod(M15);
	//ind.M30 = std::stod(M30);
	//ind.H1 = std::stod(H1);
	//ind.H4 = std::stod(H4);

	//ind.m_Slope_M1 = std::stod(M1_s);
	//ind.m_Slope_M5 = std::stod(M5_s);
	//ind.m_Slope_M15 = std::stod(M15_s);
	//ind.m_Slope_M30 = std::stod(M30_s);
	//ind.m_Slope_H1 = std::stod(H1_s);
	//ind.m_Slope_H4 = std::stod(H4_s);

	return ind;
}