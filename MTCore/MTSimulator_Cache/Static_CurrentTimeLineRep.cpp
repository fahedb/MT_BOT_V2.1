#include "Static_CurrentTimeLineRep.h"
#include "CONFIGRATIONS.H"
#include "LOGGER.H"

ConfigrationsPolicy* m_glob_confPolicy = nullptr;
OrderTracerPolicy*   m_glob_tracePolicy  = nullptr;
ScoringLayerPolicy*  m_glob_scoringPolicy = nullptr;
Static_CurrentTimeLineRep::Static_CurrentTimeLineRep()
{
    if (!m_glob_confPolicy)
        m_glob_confPolicy = Configrations::getInstance()->Get_Policy<ConfigrationsPolicy>(PolicyType::ConfigrationsPolicy);
    if (!m_glob_tracePolicy)
        m_glob_tracePolicy = Configrations::getInstance()->Get_Policy<OrderTracerPolicy>(PolicyType::OrderTracerPolicy);
    if (!m_glob_scoringPolicy)
        m_glob_scoringPolicy = Configrations::getInstance()->Get_Policy<ScoringLayerPolicy>(PolicyType::ScoringLayerPolicy);

    m_TraceTimeLine = new TimeLine(new std::vector<ChangeNode*>, true);
    m_currTimeLine = new TimeLine(new std::vector<ChangeNode*>, true);
}

void Static_CurrentTimeLineRep::Set_NewBar(ChangeNode* changeNode, bool updateCurrTimeLine)
{
    if (!changeNode) {
        Logger::getInstance().WriteLog("ChangeNode is null, cannot set new bar.", false, true, LogType::ERR);
        return;
    }
    Set_Inner_NewBar(m_TraceTimeLine, changeNode, m_glob_tracePolicy->TraceCurrTimeLineLen, true); // Update the trace time line, need to delete the memory of the oldest ChangeNode

    if (updateCurrTimeLine) Set_Inner_NewBar(m_currTimeLine, changeNode, m_glob_scoringPolicy->Pattern_CurrentTimeLineLen, false); // Update the current scan time line
}

void Static_CurrentTimeLineRep::Set_NewBar_CurrTimeLine(ChangeNode* changeNode, bool deleteOldestBarMemory) { Set_Inner_NewBar(m_currTimeLine, changeNode, m_glob_scoringPolicy->Pattern_CurrentTimeLineLen); }
void Static_CurrentTimeLineRep::Set_NewBar_TraceTimeLine(ChangeNode* changeNode, bool deleteOldestBarMemory) { Set_Inner_NewBar(m_TraceTimeLine, changeNode, m_glob_tracePolicy->TraceCurrTimeLineLen); }

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
