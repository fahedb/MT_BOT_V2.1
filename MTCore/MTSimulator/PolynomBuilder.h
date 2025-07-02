#include <vector>

class Polynom
{
private:

	int size;
	std::vector<double> m_polynom; // The index of the vector is the power, and the value is the front
public:
	void AddMemeber(double front, int upper)
	{
		if (upper > m_polynom.size())
		{
			for (int i = m_polynom.size(); i < upper; i++)
				m_polynom.push_back(0);
		}
		m_polynom[upper] += front;
	}
};

class PolynomBuilder
{
	static Polynom BuildAPolynom(std::vector<std::pair<double, double>> vec)
	{

	}



};