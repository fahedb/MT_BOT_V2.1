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
protected:
	BasePricesTable(SQLHENV& henv, SQLHDBC& hdbc, std::string tableName) : BTables(henv, hdbc) {
		m_tableName = tableName;
	}
	BasePricesTable(std::string filename, std::string tableName) : BTables(filename)
	{
		m_tableName = tableName;
	}
	BasePricesTable(std::string tableName) : BTables() // Constuctor for Json use
	{
		m_tableName = tableName;
	}
public: 
	///////////////////////////////////////////////////
	////// All functions to get selected counts////////
	///////////////////////////////////////////////////

	std::vector<SQLSelectionNode*>* GetSelected() { return GetSelectedRows(); }
	std::vector<FileSelectionNode*>* GetSelectedFromFile() { return GetSelectedRowsFromFile(); }
	JsonSelectionNode* GetNextJsonEntity_MultiThreaded(int threadIdx) { return GetNextJsonSelection_MultiThreaded(threadIdx); }
	virtual int GetSelected_Count_MultiThreaded(int threadIdx) { return BTables::Get_SelectedCount_MultiThreaded(threadIdx); }

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:
	void Set_DATA_DB_FileName(std::string fileName) { Set_DBFileName(fileName); }
	void Set_MultiThreadedRead() { Set_AsMultiThread_Read(); }

protected:

	void ClearAll();
public:
	std::string Get_TableName() { return m_tableName; }
	
public:
	///////////////////////////////////////////////////
	////// All functions to select from DB ////////////
	///////////////////////////////////////////////////

	bool SelectDataFromFile() { return SelectFromFile(GetColsNum()); }
	bool SelectDataFromJsonFile() { return SelectFromJsonFile(); }

	virtual void FreeStyleQuery(std::string sqlCommand) { SelectFromDB(sqlCommand, tableCols()); }
	virtual bool FreeStyleCommand(std::string sqlCommand) { return UpdateDB(sqlCommand); }
	virtual void RemoveSelect() { RemoveSelection(); }

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

protected:
	int index = 0; // Index of selection, used in get next entity functions
	virtual std::string GetDB_PhysicalFileName() override { return	Configrations::getInstance()->Get_ConfigValue(DBFILE_DEFUALT_DB_FILE); }
	virtual int tableCols() { return HistoryPricesTablesCols; } // To get columns number, Not used any more


protected:
	///////////////////////////////////////////////////
	////// All functions to get selected entities /////
	///////////////////////////////////////////////////

	virtual SQLSelectionNode* GetPrevEnt(); // Getting prev entity from SQL DB selection, if there is one 
	virtual SQLSelectionNode* GetNextEnt(); // Getting next entity from SQL DB selection, if there is one 

	virtual FileSelectionNode* GetPrevFileEnt(); // Getting prev entity from File selection, if there is one 
	virtual FileSelectionNode* GetNextFileEnt(); // Getting next entity from File selection, if there is one 

	JsonSelectionNode* GetNextJsonEntity() { return GetNextJsonSelection(); } // Getting prev entity from json selection (file) , if there is one 

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

public:
	virtual void Insert_NewCandleBody(JsonSelectionNode* js) { Insert_NewBody_to_queue(js); }

	// Getting change node in multi thread mode, Pay attention that after done getting all change nodes, need to clear all json objects by calling RemoveSelect() !!
	virtual ChangeNode* GetNextChangeNode_MultiThreaded(int threadIdx); 

	// Getting change node in one thread mode
	ChangeNode* GetNextChangeNode(); 


private:
	virtual _Points GetPointPlace() { return _Points::_Points_DEFUALT; } // Point place for the Price, every pair is diffrent
	virtual int GetColsNum() { return DEF_COLUMN_NUM_DEFUALT; } // Virtual function to return the columns number, need to overloaded by the derived class
};

#endif
