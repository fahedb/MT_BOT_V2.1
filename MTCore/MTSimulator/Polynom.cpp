#include "Polynom.h"
#include "LOGGER.H"

void Polynom::AddMember(double front, int upper)
{
	if (upper < 0)
	{
		Logger::getInstance().WriteLog("Polynom::AddMember - upper is negative, cannot add member", true, true, LogType::ERR);
		return; // Do not add if upper is negative
	}

	if (upper > m_polynom.size() - 1)
	{
		for (int i = m_polynom.size() - 1; i < upper; i++)
			m_polynom.push_back(0);
	}
	m_polynom[upper] = front;
	size = static_cast<int>(m_polynom.size());
}
