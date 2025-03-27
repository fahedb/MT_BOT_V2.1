#include "BTables.h"
#include "ChangeData.h"
#include "ProcessData.h"
#include "Configrations.h"
#include "DefinedTables.h"
#include "CandleData.h"
#pragma once

#ifndef BaseHistoryPricesTable_HEADER
#define BaseHistoryPricesTable_HEADER
#define HistoryPricesTablesCols 4


class BasePricesTable : BTables
{
private:
	std::string m_tableName;

	std::string m_FPriceDateTime = "";
	float m_FOpenPrice = 0;
	float m_FClosePrice = 0;

	bool m_FActionSubmitted = 0;

	std::string m_IPriceDateTime = "";
	std::string m_IPriceDateTimeRelational = "";
	std::string m_IPriceDateTimeRelation = "";
	float m_IBuyPrice = 0;
	float m_ISellPrice = 0;
	bool m_IActionSubmitted = 0;
	bool m_PriceDateTime_AscDescSort;
	bool SortResults;

	ChangeNode m_PrevChageNode;
protected:
	BasePricesTable(SQLHENV& henv, SQLHDBC& hdbc, std::string tableName) : BTables(henv, hdbc) {
		m_tableName = tableName;
		SortResults = false;
	}
	BasePricesTable(std::string filename, std::string tableName) : BTables(filename)
	{
		m_tableName = tableName;
		SortResults = false;
	}
	BasePricesTable(std::string tableName) : BTables() // Constuctor for Json use
	{
		m_tableName = tableName;
		SortResults = false;
	}
protected:
	
	std::vector<SQLSelectionNode*>* GetSelected() { return GetSelectedRows(); }
	std::vector<FileSelectionNode*>* GetSelectedFromFile() { return GetSelectedRowsFromFile(); }
	JsonSelectionNode* GetNextJsonEntity() { return GetNextJsonSelection(); }
	JsonSelectionNode* GetNextJsonEntity_MultiThreaded(int threadIdx) { return GetNextJsonSelection_MultiThreaded(threadIdx); }
public:
	void Set_DATA_DB_FileName(std::string fileName) { Set_DBFileName(fileName); }
	void Set_MultiThreadedRead() { Set_AsMultiThread_Read(); }
protected:

	void FPriceDateTime(std::string tm) { m_FPriceDateTime = tm; }
	void FBuyPrice(float bp) { m_FOpenPrice = bp; }
	void FSellPrice(float sp) { m_FClosePrice = sp; }
	void FActionSubmitted(bool as) { m_FActionSubmitted = as; }

protected:
	void IPriceDateTime(std::string tm) {
		m_IPriceDateTime = "PriceDateTime";
		m_IPriceDateTime += "=" + tm;
	}
	void IPriceDateTime(std::string tm, std::string relation) {
		m_IPriceDateTime = "PriceDateTime";
		m_IPriceDateTime += relation + tm;
	}
	void IBuyPrice(float bp) { m_IBuyPrice = bp; }
	void ISellPrice(float sp) { m_ISellPrice = sp; }
	void IActionSubmitted(bool as) { m_IActionSubmitted = as; }
	void B_SortPriceDateTime(bool asc)
	{
		m_PriceDateTime_AscDescSort = asc;
		SortResults = true;
	}

	bool SubmitInsert();
	bool SubmitQuery();
	void ClearAll();
	void ClearQuery();
	bool FreeStyleQueryCommand(std::string sqlCommamd) { return SelectFromDB(sqlCommamd, tableCols()); }
	void ClearInsert();
	void RemoveSelectedRows() { RemoveSelection(); }
private:
	void addFilter(const std::string& filter, std::string& query);
	void addSort(std::string& query);

public:
	std::string Get_TableName() { return m_tableName; }
	// pure Virual Functions
public:
	virtual void InitValue_PriceDateTime(std::string pdt) { FPriceDateTime(pdt); }
	virtual void InitValue_BuyPrice(float bp) { FBuyPrice(bp); }
	virtual void InitValue_SellPrice(float sl) { FSellPrice(sl); }
	virtual void InitValue_ActionSubmitted(int as) { FActionSubmitted(as); }


	virtual void AddFilter_PriceDateTime(std::string pdt) { IPriceDateTime(pdt); }
	virtual void AddFilter_PriceDateTime(std::string pdt, std::string relation) { IPriceDateTime(pdt, relation); }

	virtual void AddFilter_BuyPrice(float bp) { IBuyPrice(bp); }
	virtual void AddFilter_SellPrice(float sp) { ISellPrice(sp); }
	virtual void AddFilter_ActionSubmitted(bool as) { IActionSubmitted(as); }
	virtual void AddSort_PriceDateTime(bool asc) { B_SortPriceDateTime(asc); }

	virtual void StartInsertCommand() { ClearInsert(); }
	virtual void StartQueryCommand() { ClearQuery(); index = 0; }
	bool SelectDataFromFile() { return SelectFromFile(GetColsNum()); }
	bool SelectDataFromJsonFile() { return SelectFromJsonFile(); }
	virtual void FreeStyleQuery(std::string sqlCommand) { FreeStyleQueryCommand(sqlCommand); }
	virtual bool FreeStyleCommand(std::string sqlCommand) { return DBCommand(sqlCommand); }
	virtual bool FreeStyleUpdateCommand(std::string sqlCommand) { return UpdateDB(sqlCommand); }
	virtual int GetSelected_Count() { return BTables::Get_SelectedCount(); }
	virtual int GetSelected_Count_MultiThreaded(int threadIdx) { return BTables::Get_SelectedCount_MultiThreaded(threadIdx); }
	virtual void RunQuery() { SubmitQuery(); }
	virtual void RunInsert() { SubmitInsert(); }
	virtual void RemoveSelect() { RemoveSelectedRows(); }
	virtual void Insert_NewCandleBody(JsonSelectionNode* js) { Insert_NewBody_to_queue(js); }
protected:
	int index = 0;
	virtual std::string GetDB_PhysicalFileName() override { return	Configrations::getInstance()->Get_ConfigValue(DBFILE_DEFUALT_DB_FILE); }
	virtual int tableCols() {
		return HistoryPricesTablesCols;
	}


public:
	virtual SQLSelectionNode* GetPrevEnt();
	virtual FileSelectionNode* GetPrevFileEnt();

	virtual SQLSelectionNode* GetNextEnt();
	virtual FileSelectionNode* GetNextFileEnt();

	virtual ChangeNode *GetNextChangeNode(ChangeNode* prev_chN=nullptr);
	virtual ChangeNode* GetNextChangeNode_MultiThreaded(int threadIdx);

private:
	virtual _Points GetPointPlace() { return _Points::_Points_DEFUALT; }
	virtual ChangeNode * BuildChangeNode(SQLSelectionNode*, ChangeNode* prev_CHn = nullptr);
	virtual ChangeNode * BuildChangeNode(FileSelectionNode*, ChangeNode* prev_CHn = nullptr);
	virtual ChangeNode* BuildChangeNode(JsonSelectionNode*);
	//virtual ChangeNode * BuildChangeNode(FileSelectionNode* col);
	virtual int GetColsNum() { return DEF_COLUMN_NUM_DEFUALT; }
};

#endif
