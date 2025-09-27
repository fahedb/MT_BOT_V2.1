#include <string>

#pragma once
enum class CoinsPair
{
	XAUUSD = 0,
	EURUSD = 1,
	NONE = -1

};

class CoinsPairUtils
{
public:
	static std::string Get_PairType_str(CoinsPair pair)
	{
		switch (pair)
		{
		case CoinsPair::XAUUSD:
			return "XAUUSD";
		case CoinsPair::EURUSD:
			return "EURUSD";
		default:
			return "";
		}
	}
};