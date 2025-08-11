#pragma once
#include <vector>
#include "DBMap.h"
#include "ChangeData.h"
#include "LOGGER.H"

typedef std::pair<long long, long long> TimeLineSegment;

class TimeLine
{

public:

    TimeLine(TimeLine&& other) noexcept
    {
        m_TL = other.m_TL;
        m_DB_Map = other.m_DB_Map;
        m_selfOwnership = other.m_selfOwnership;

        other.m_TL = nullptr;
        other.m_selfOwnership = false;
    }
    TimeLine(TimeLine* other) noexcept
    {
        m_TL = other->m_TL;
        m_DB_Map = other->m_DB_Map;
        m_selfOwnership = other->m_selfOwnership;
        other->m_TL = nullptr;
        other->m_selfOwnership = false;
	}

    TimeLine(std::vector<ChangeNode*>* TL,bool ownership = false) {
        m_TL = TL;
        Init_TimeLineDB();
        m_selfOwnership = ownership;
    }

    TimeLine(std::vector<ChangeNode*> TL) {
        m_TL = new std::vector<ChangeNode*>(TL);
        Init_TimeLineDB();
        m_selfOwnership = true;
    }

    TimeLine(const TimeLine& other)
    {
        m_selfOwnership = true; 
        m_TL = new std::vector<ChangeNode*>(*other.m_TL);
        m_DB_Map = other.m_DB_Map;
		Logger::getInstance().WriteLog("TimeLine copy constructor called, this is not recommended, please use move semantics instead.", false, true, LogType::ERR);
    }
    TimeLine& operator=(const TimeLine&) = delete;
    size_t Get_TimeLineLen() { if (m_TL) return m_TL->size(); else  return -1; }

    ~TimeLine()
    {
        if (m_selfOwnership && m_TL != nullptr) {
            delete m_TL;
            m_TL = nullptr;
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
            long long timeKey = m_TL->at(i)->point_DateTimeLong;
            if (!m_DB_Map.ContainsKey(timeKey)) {
                m_DB_Map.InsetIntoMap(timeKey, i);
            }
            else {
                Logger::getInstance().WriteLog("!!!!! Duplicate point_DateTime in DB: " + std::to_string(timeKey) + " !!!!!!", false, true, LogType::ERR);
            }
        }
    }


};

