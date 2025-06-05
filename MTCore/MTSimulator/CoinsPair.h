#pragma once
enum class CoinsPair
{
	XAUUSD = 0,
	EURUSD = 1,
	NONE = -1

};


class CoinsPairUtils {

public:
	static CoinsPair Get_CoinsPair_FromString(const std::string& pair)
	{
		if (pair == "XAUUSD") return CoinsPair::XAUUSD;
		if (pair == "EURUSD") return CoinsPair::EURUSD;
		return CoinsPair::NONE;
	}
	static std::string Get_CoinsPair_String(CoinsPair pair)
	{
		switch (pair)
		{
		case CoinsPair::XAUUSD:
			return "XAUUSD";
		case CoinsPair::EURUSD:
			return "EURUSD";
		default:
			return "NONE";
		}
	}

};