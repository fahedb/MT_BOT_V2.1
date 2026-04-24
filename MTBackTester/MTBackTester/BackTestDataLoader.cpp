#include "BackTestDataLoader.h"
#include "TimeLineBuilder.h"

void BackTestDataLoader::Load_BackTestData(const std::string& pathToFileDir, std::vector<ChangeNode>* historyMemoryPool)
{
	IndicatorStorage* indStorage = nullptr;
	std::vector<ChangeNode*> history;
	XAUUSD_OneMinTable table(pathToFileDir, historyMemoryPool, indStorage);
	HistoryTimeLineLoader historyLoader(m_backTestData, &table, pathToFileDir);
	historyLoader.Run(pathToFileDir);
	TimeLineBuilder::Build_TimeLineDB(&m_backTestData, m_dateTimeToIdxMap);
}
