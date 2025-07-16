#include "Polynom.h"

void Polynom::SetPolynom(const std::vector<double>& polynom)
{
	m_polynom = polynom;
	size = static_cast<int>(polynom.size());
}

void Polynom::AddMemeber(double front, int upper)
{
	if (upper > m_polynom.size())
	{
		for (int i = m_polynom.size(); i < upper; i++)
			m_polynom.push_back(0);
	}
	m_polynom[upper] = front;
}
