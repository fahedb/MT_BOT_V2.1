#pragma once
#include <vector>
#include "DBMap.h"
#include "ChangeData.h"

typedef std::pair<long long, long long> TimeLineSegment;

class TimeLine
{

public:

    TimeLine(TimeLine&& other) noexcept;
    TimeLine(TimeLine* other) noexcept;

    TimeLine(std::vector<ChangeNode*>* TL,bool ownership = false);

    TimeLine(std::vector<ChangeNode*> TL);

    TimeLine(const TimeLine& other);
    TimeLine& operator=(const TimeLine&) = delete;
    size_t Get_TimeLineLen() { if (m_TL) return m_TL->size(); else  return -1; }

    ~TimeLine();
    void Set_SelfOwnership(bool selfOwner) { m_selfOwnership = selfOwner; }

    std::vector<ChangeNode*>* Get_TimeLine() { return m_TL; } 
    DBMap* Get_DBMap() { return &m_DB_Map; }

private:

    std::vector<ChangeNode*>* m_TL;
    DBMap m_DB_Map;
    bool m_selfOwnership;
    // Private constructor to prevent instantiation
    
    void Init_TimeLineDB();


};

