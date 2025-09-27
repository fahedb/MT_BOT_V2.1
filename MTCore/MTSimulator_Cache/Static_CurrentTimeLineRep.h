#include "TimeLine.h"

#pragma once

/// <summary>
/// Class to save current timeline 
/// And long current timeLine (last X bars (by a config) )
/// </summary>

class Static_CurrentTimeLineRep
{

public:

    Static_CurrentTimeLineRep();

    void Set_NewBar(ChangeNode* changeNode, bool updateCurrTimeLine = false);

    void Set_NewBar_CurrTimeLine(ChangeNode* changeNode, bool deleteOldestBarMemory = false);
    void Set_NewBar_TraceTimeLine(ChangeNode* changeNode, bool deleteOldestBarMemory = false);

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
    void Set_Inner_NewBar(TimeLine* tl, ChangeNode* changeNode, size_t maxLen, bool deleteOldestBarMemory = false);
};

