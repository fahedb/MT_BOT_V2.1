#include "Polynom.h"


void Polynom::AddMemeber(double front, int upper)
{
	if (upper > m_polynom.size() - 1)
	{
		for (int i = m_polynom.size() - 1; i < upper; i++)
			m_polynom.push_back(0);
	}
	m_polynom[upper] = front;
	size = static_cast<int>(m_polynom.size());
}
