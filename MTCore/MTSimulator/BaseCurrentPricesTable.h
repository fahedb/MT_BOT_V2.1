#pragma once
#include "BTables.h"

class BaseCurrentPricesTable:BTables
{
private:
	std::string m_tableName;

	std::string m_FPriceDateTime = "";
	float m_FBuyPrice = 0;
	float m_FSellPrice = 0;
	bool m_FActionSubmitted = 0;

	std::string m_IPriceDateTime = "";
	float m_IBuyPrice = 0;
	float m_ISellPrice = 0;
	bool m_IActionSubmitted = 0;
	bool m_PriceDateTime_AscDescSort;
	bool SortResults;
protected:
	BaseCurrentPricesTable(SQLHENV henv, SQLHDBC hdbc, std::string tableName) : BTables(henv, hdbc) {
		m_tableName = tableName;
		SortResults = false;
	}

protected:
	std::vector<SQLSelectionNode*>* GetSelected() { return GetSelectedRows(); }
protected:
	void FPriceDateTime(std::string tm) { m_FPriceDateTime = tm; }
	void FBuyPrice(float bp) { m_FBuyPrice = bp; }
	void FSellPrice(float sp) { m_FSellPrice = sp; }
	void FActionSubmitted(bool as) { m_FActionSubmitted = as; }

protected:
	void IPriceDateTime(std::string tm) { m_IPriceDateTime = tm; }
	void IBuyPrice(float bp) { m_IBuyPrice = bp; }
	void ISellPrice(float sp) { m_ISellPrice = sp; }
	void IActionSubmitted(bool as) { m_IActionSubmitted = as; }
	void B_SortPriceDateTime(bool asc)
	{
		m_PriceDateTime_AscDescSort = asc;
		SortResults = true;
	}

	bool SubmitInsert();
	bool FreeStyleQueryCommand(std::string sqlCommamd) { SelectFromDB(sqlCommamd, HistoryPricesTablesCols); }
	bool SubmitQuery();
	void ClearAll();
	void ClearQuery();
	void ClearInsert();

private:
	void addFilter(const std::string& filter, std::string& query);
	void addSort(std::string& query);



};

