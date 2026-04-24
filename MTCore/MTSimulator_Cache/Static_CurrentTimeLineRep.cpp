#include "Static_CurrentTimeLineRep.h"
#include "LOGGER.H"

Static_CurrentTimeLineRep* Static_CurrentTimeLineRep::instancePtr = nullptr;

Static_CurrentTimeLineRep::Static_CurrentTimeLineRep(int Pattern_CurrentTimeLineLen, int TraceCurrTimeLineLen, bool memoryDeleteAfterDoneUsing)
{

    m_TraceTimeLine = new TimeLine(new std::vector<ChangeNode*>, true);
    m_currTimeLine = new TimeLine(new std::vector<ChangeNode*>, true);

	m_Pattern_CurrentTimeLineLen = Pattern_CurrentTimeLineLen;
	m_TraceCurrTimeLineLen = TraceCurrTimeLineLen;
	m_memoryDeleteAfterDoneUsing = memoryDeleteAfterDoneUsing;
}

void Static_CurrentTimeLineRep::Init(int Pattern_CurrentTimeLineLen, int TraceCurrTimeLineLen, bool memoryDeleteAfterDoneUsing)
{
    if (instancePtr != nullptr) {
        std::cout << "Static_CurrentTimeLineRep is already initialized." << std::endl;
        return;
    }
    instancePtr = new Static_CurrentTimeLineRep(Pattern_CurrentTimeLineLen, TraceCurrTimeLineLen, memoryDeleteAfterDoneUsing);
}

void Static_CurrentTimeLineRep::Set_NewBar(ChangeNode* changeNode, bool updateCurrTimeLine)
{
    if (!changeNode) {
        Logger::getInstance().WriteLog("ChangeNode is null, cannot set new bar.", false, true, LogType::ERR);
        return;
    }
    Set_Inner_NewBar(m_TraceTimeLine, changeNode, m_TraceCurrTimeLineLen, m_memoryDeleteAfterDoneUsing); // Update the trace time line, need to delete the memory of the oldest ChangeNode

    if (updateCurrTimeLine) Set_Inner_NewBar(m_currTimeLine, changeNode, m_Pattern_CurrentTimeLineLen, m_memoryDeleteAfterDoneUsing); // Update the current scan time line
}

void Static_CurrentTimeLineRep::Set_NewBar_CurrTimeLine(ChangeNode* changeNode, bool deleteOldestBarMemory) { Set_Inner_NewBar(m_currTimeLine, changeNode, m_Pattern_CurrentTimeLineLen, m_memoryDeleteAfterDoneUsing); }
void Static_CurrentTimeLineRep::Set_NewBar_TraceTimeLine(ChangeNode* changeNode, bool deleteOldestBarMemory) { Set_Inner_NewBar(m_TraceTimeLine, changeNode, m_TraceCurrTimeLineLen, m_memoryDeleteAfterDoneUsing); }

void Static_CurrentTimeLineRep::Set_Inner_NewBar(TimeLine* tl, ChangeNode* changeNode, size_t maxLen, bool deleteOldestBarMemory)
{

    if (tl->Get_TimeLineLen() >= maxLen)
    {
        while (tl->Get_TimeLineLen() >= maxLen)
        {
            ChangeNode* oldestBar = tl->Get_TimeLine()->at(0);
            tl->Get_TimeLine()->erase(tl->Get_TimeLine()->begin());

            if (deleteOldestBarMemory)
                delete oldestBar;
        }
    }
    tl->Get_TimeLine()->push_back(changeNode);

}
