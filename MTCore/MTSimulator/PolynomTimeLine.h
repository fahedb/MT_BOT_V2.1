#include "TimeLine.h"
#include "PolynomChangeNodePackage.h"
#include "MemoryUtils.h"

#pragma once

class PolynomTimeLine
{
public:
	// Constructor for PolynomTimeLine with ownership of PolynomChangeNodePackage and TimeLine
	PolynomTimeLine(TimeLineSegment segment, PolynomChangeNodePackage* polynomPackage, TimeLine* timeLine, MemoryActionType mAT,bool selfOwnerShip = true)
	{
		m_segment = segment;
		m_MemoryActionType = mAT; // If true, this class owns the memory of the PolynomChangeNodePackage and TimeLine, otherwise they are managed externally
		m_selfOwnership = selfOwnerShip; // If true, this class will take ownership of the memory, otherwise it will not

		if(m_MemoryActionType == MemoryActionType::MOVE)
		{
			if (polynomPackage) // Move poylnomPackage
				m_polynomPackage = new PolynomChangeNodePackage(polynomPackage); // Move the PolynomChangeNodePackage
			if (timeLine) // Move timeLine
				m_timeLine = new TimeLine(timeLine); // Move the TimeLine
		}
		else if (m_MemoryActionType == MemoryActionType::DEEP_COPY)
		{
			m_selfOwnership = true; // This class will take ownership of the memory
			if (polynomPackage) // Deep copy PolynomChangeNodePackage
				m_polynomPackage = new PolynomChangeNodePackage(*polynomPackage);
			if (timeLine) // Deep copy TimeLine
				m_timeLine = new TimeLine(*timeLine);
		}
		else
		{
			m_polynomPackage = polynomPackage; // Copying the pointer, not the object
			m_timeLine = timeLine; // Copying the pointer, not the object
			Logger::getInstance().WriteLog("PolynomTimeLine constructor called with MemoryActionType::NONE, this is not recommended, please use move semantics instead.", false, true, LogType::ERR);
		}
	}

	// Move constructor
	PolynomTimeLine(PolynomTimeLine&& other) noexcept
	{
		m_segment = other.m_segment;
		m_polynomPackage = other.m_polynomPackage;
		m_timeLine = other.m_timeLine;
		m_selfOwnership = other.m_selfOwnership;
		other.m_polynomPackage = nullptr; // Transfer ownership
		other.m_timeLine = nullptr; // Transfer ownership
	}
	// Copy constructor (not recommended, use move semantics instead)
	PolynomTimeLine(const PolynomTimeLine& other)
	{
		m_segment = other.m_segment;
		m_polynomPackage = other.m_polynomPackage; // Copying the pointer, not the object
		m_timeLine = other.m_timeLine; // Copying the pointer, not the object
		m_selfOwnership = false; // This class does not own the memory of the PolynomChangeNodePackage and TimeLine
		Logger::getInstance().WriteLog("PolynomTimeLine copy constructor called, this is not recommended, please use move semantics instead.", false, true, LogType::ERR);
	}

	~PolynomTimeLine()
	{
		if (m_selfOwnership)
		{
			if (m_polynomPackage) // Check if the PolynomChangeNodePackage is not null
				delete m_polynomPackage; // Delete the PolynomChangeNodePackage if this class owns it
			m_polynomPackage = nullptr;
			if (m_timeLine) // Check if the TimeLine is not null
				delete m_timeLine; // Delete the TimeLine if this class owns it
			m_timeLine = nullptr;
		}
	}
	TimeLine* Get_TimeLine() { return m_timeLine; }
	PolynomChangeNodePackage* Get_PolynomPackage() { return m_polynomPackage; }
	TimeLineSegment Get_Segment() { return m_segment; }
private:
	TimeLineSegment m_segment;
	PolynomChangeNodePackage* m_polynomPackage = nullptr; // Polynom package for this segment
	TimeLine* m_timeLine = nullptr; // TimeLine for this segment
	MemoryActionType m_MemoryActionType = MemoryActionType::NONE; 
	bool m_selfOwnership = false; // If true, this class owns the memory of the PolynomChangeNodePackage and TimeLine
};

