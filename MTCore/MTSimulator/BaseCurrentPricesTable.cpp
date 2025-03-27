#include "BaseCurrentPricesTable.h"


void BaseCurrentPricesTable::ClearAll()
{
	m_FPriceDateTime = "";
	m_FBuyPrice = 0;
	m_FSellPrice = 0;
	m_FActionSubmitted = 0;

	m_IPriceDateTime = "";
	m_IBuyPrice = 0;
	m_ISellPrice = 0;
	m_IActionSubmitted = 0;
	SortResults = false;
	RemoveSelection();
}

void BaseCurrentPricesTable::ClearQuery()
{
	m_IPriceDateTime = "";
	m_IBuyPrice = 0;
	m_ISellPrice = 0;
	m_IActionSubmitted = 0;
	SortResults = false;
}

void BaseCurrentPricesTable::ClearInsert()
{
	m_FPriceDateTime = "";
	m_FBuyPrice = 0;
	m_FSellPrice = 0;
	m_FActionSubmitted = 0;
}


void BaseCurrentPricesTable::addFilter(const std::string& filter, std::string& query)
{
	int idx = query.find("WHERE");
	if (idx >= 0)
		query += " AND ";
	else
		query += " WHERE ";
	query += (filter);
}
void BaseCurrentPricesTable::addSort(std::string& query)
{
	if (m_PriceDateTime_AscDescSort)
		query += " order by PriceDateTime asc";
	else
		query += " order by PriceDateTime desc";
}
bool BaseCurrentPricesTable::SubmitQuery()
{
	std::string querySQLCommand = "SELECT * FROM " + m_tableName + " (NOLOCK) ";
	bool withConditions = false;

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
	SelectFromDB(querySQLCommand, HistoryPricesTablesCols);
	return true;
}

bool BaseCurrentPricesTable::SubmitInsert()
{
	std::string insertSQLCommand = "INSERT INTO " + m_tableName + " (PriceDateTime, BuyPrice,SellPrice,ActionSubmitted) VALUES('" + m_FPriceDateTime + "'," + std::to_string(m_FBuyPrice) + "," + std::to_string(m_FSellPrice) + "," + std::to_string(m_FActionSubmitted) + ")";
	bool result = InsertIntoDB(insertSQLCommand);
	ClearInsert();

	return result;
}