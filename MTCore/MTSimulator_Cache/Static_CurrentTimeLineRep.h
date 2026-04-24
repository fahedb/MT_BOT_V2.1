#pragma once
#include "TimeLine.h"


/// <summary>
/// Class to save current timeline 
/// And long current timeLine (last X bars (by a config) )
/// </summary>

class Static_CurrentTimeLineRep
{
private:
	static Static_CurrentTimeLineRep* instancePtr; // Pointer to the singleton instance

	int m_Pattern_CurrentTimeLineLen;
	int m_TraceCurrTimeLineLen;
    bool m_memoryDeleteAfterDoneUsing;
    Static_CurrentTimeLineRep(int Pattern_CurrentTimeLineLen, int TraceCurrTimeLineLen, bool memoryDeleteAfterDoneUsing = true);
public:
    static void Init(int Pattern_CurrentTimeLineLen, int TraceCurrTimeLineLen, bool memoryDeleteAfterDoneUsing = true);


    void Set_NewBar(ChangeNode* changeNode, bool updateCurrTimeLine = false);

    void Set_NewBar_CurrTimeLine(ChangeNode* changeNode, bool deleteOldestBarMemory = false);
    void Set_NewBar_TraceTimeLine(ChangeNode* changeNode, bool deleteOldestBarMemory = false);

    TimeLine* Get_CurrTimeLine() { return m_currTimeLine; }
    TimeLine* Get_TraceTimeLine() { return m_TraceTimeLine; }


    Static_CurrentTimeLineRep(const Static_CurrentTimeLineRep&) = delete;
    Static_CurrentTimeLineRep& operator=(const Static_CurrentTimeLineRep&) = delete;

    // Static method to get the instance of the Singleton
    static Static_CurrentTimeLineRep* getInstance() {
        if(instancePtr == nullptr) {
            throw std::runtime_error("Static_CurrentTimeLineRep Singleton not initialized");
        }
        return instancePtr;
    }
private:
    TimeLine* m_currTimeLine;
    TimeLine* m_TraceTimeLine;
    void Set_Inner_NewBar(TimeLine* tl, ChangeNode* changeNode, size_t maxLen, bool deleteOldestBarMemory = false);
};

