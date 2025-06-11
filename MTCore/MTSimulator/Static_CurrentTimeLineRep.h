#include "TimeLine.h"
#include "CONFIGRATIONS.H"

#pragma once

/// <summary>
/// Class to save current timeline 
/// And long current timeLine (last X bars (by a config) )
/// </summary>

class Static_CurrentTimeLineRep
{

public:

    Static_CurrentTimeLineRep()
    {
        m_policy = Configrations::getInstance()->Get_Policy();
        m_TraceTimeLine = new TimeLine(new std::vector<ChangeNode*>, true);
        m_currTimeLine = new TimeLine(new std::vector<ChangeNode*>, true);
    }

    void Set_NewBar(ChangeNode* changeNode, bool updateCurrTimeLine = false)
    {
		if (!changeNode) {
			Logger::getInstance().WriteLog("ChangeNode is null, cannot set new bar.", false, true, LogType::ERR);
			return;
		}
        Set_Inner_NewBar(m_TraceTimeLine, changeNode, m_policy->TraceCurrTimeLineLen,true); // Update the trace time line, need to delete the memory of the oldest ChangeNode

        if (updateCurrTimeLine) Set_Inner_NewBar(m_currTimeLine, changeNode, m_policy->Pattern_CurrentTimeLineLen, false); // Update the current scan time line
    }

    void Set_NewBar_CurrTimeLine(ChangeNode* changeNode, bool deleteOldestBarMemory = false) { Set_Inner_NewBar(m_currTimeLine, changeNode, m_policy->Pattern_CurrentTimeLineLen); }
    void Set_NewBar_TraceTimeLine(ChangeNode* changeNode, bool deleteOldestBarMemory = false) { Set_Inner_NewBar(m_TraceTimeLine, changeNode, m_policy->TraceCurrTimeLineLen); }

    TimeLine* Get_CurrTimeLine() { return m_currTimeLine; }
    TimeLine* Get_TraceTimeLine() { return m_TraceTimeLine; }


    Static_CurrentTimeLineRep(const Static_CurrentTimeLineRep&) = delete;
    Static_CurrentTimeLineRep& operator=(const Static_CurrentTimeLineRep&) = delete;

    // Static method to get the instance of the Singleton
    static Static_CurrentTimeLineRep* getInstance() {
        static Static_CurrentTimeLineRep instance; // Guaranteed to be destroyed and instantiated on first use
        return &instance;
    }
private:
    TimeLine* m_currTimeLine;
    TimeLine* m_TraceTimeLine;
    ConfigrationsPolicy* m_policy;
    void Set_Inner_NewBar(TimeLine* tl, ChangeNode* changeNode, size_t maxLen, bool deleteOldestBarMemory = false)
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
};

