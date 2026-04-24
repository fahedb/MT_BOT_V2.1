#pragma once
#include "ChangeData.h"
#include "XAUUSD_OneMinTable.h"
#include "HistoryTimeLineLoader.h"

class BackTestDataLoader
{
private:
	std::vector<ChangeNode*> m_backTestData;
	DBMap* m_dateTimeToIdxMap = nullptr;
public :
	void Load_BackTestData(const std::string& pathToFileDir, std::vector<ChangeNode>* historyMemoryPool);
	void Get_BackTestDataCopy(std::vector<ChangeNode*>* outData) { *outData = m_backTestData; }
	std::vector<ChangeNode*>* Get_BackTestDataPointer() { return &m_backTestData; }

	DBMap* Get_DateTimeToIdxMap() { return m_dateTimeToIdxMap; }
};

