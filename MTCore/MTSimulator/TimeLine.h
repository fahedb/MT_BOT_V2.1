#pragma once
#include <vector>
#include "DBMap.h"
#include "ChangeData.h"
#include "LOGGER.H"

class TimeLine
{

public:
    // Deleted copy constructor and assignment operator to prevent copies
    
    TimeLine(std::vector<ChangeNode*>* TL,bool ownership = false) {
        m_TL = TL;
        Init_TimeLineDB();
        m_selfOwnership = ownership;
    }

    ~TimeLine()
    {
        if (m_selfOwnership)
        {
            m_TL->clear();
            delete m_TL;
            m_DB_Map.~DBMap();
        }
    }
    void Set_SelfOwnership(bool selfOwner) { m_selfOwnership = selfOwner; }

    std::vector<ChangeNode*>* Get_TimeLine() { return m_TL; }
    DBMap* Get_DBMap() { return &m_DB_Map; }

private:

    std::vector<ChangeNode*>* m_TL;
    DBMap m_DB_Map;
    bool m_selfOwnership;
    // Private constructor to prevent instantiation
    
    void Init_TimeLineDB() {

        for (int i = 0; i < m_TL->size(); i++)
        {
            const std::string& timeKey = m_TL->at(i)->point_DateTime;
            if (!m_DB_Map.ContainsKey(timeKey)) {
                m_DB_Map.InsetIntoMap(timeKey, i);
            }
            else {
                Logger::getInstance().WriteLog("!!!!! Duplicate point_DateTime in DB: " + timeKey + " !!!!!!", false, true, LogType::SPECIAL_LOG);
            }
        }
    }


};

