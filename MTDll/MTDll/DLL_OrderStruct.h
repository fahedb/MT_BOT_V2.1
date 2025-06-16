#include "pch.h"
#include <string>



// CPP enum def side
enum DLL_OrderType { DLL_ORDER_TYPE_LONG = 0, DLL_ORDER_TYPE_SHORT = 1, DLL_ORDER_TYPE_NONE = -1 };
enum DLL_OrderStatus { DLL_ORDER_STATUS_OPEN = 0, DLL_ORDER_STATUS_CLOSED = 1, DLL_ORDER_STATUS_NONE= -1 };
enum DLL_CoinsPair { DLL_COINS_PAIR_XAUUSD = 0, DLL_COINS_PAIR_EURUSD = 1, DLL_COINS_PAIR_NONE = -1 };


typedef struct DLL_OrderStruct {
	DLL_OrderType type;
	DLL_OrderStatus status;
	DLL_CoinsPair pair;
	int seqNum;
	int stopLoss;
	int takeProfit;
} DLL_OrderStruct, * DLL_OrderStruct_P;

/// <summary>
/// DLL ENUM UTILS
/// </summary>

class DLL_OrderTypeUtils
{
public:
	static DLL_OrderType Get_OrderType_FromString(const std::string& type)
	{
		if (type == "LONG")
			return DLL_OrderType::DLL_ORDER_TYPE_LONG;
		else if (type == "SHORT")
			return DLL_OrderType::DLL_ORDER_TYPE_SHORT;
		else
			return DLL_OrderType::DLL_ORDER_TYPE_NONE;
	}
	static std::string Get_OrderType_String(DLL_OrderType type)
	{
		switch (type)
		{
		case DLL_OrderType::DLL_ORDER_TYPE_LONG:
			return "LONG";
		case DLL_OrderType::DLL_ORDER_TYPE_SHORT:
			return "SHORT";
		default:
			return "NONE";
		}
	}
};


class DLL_OrderStatusUtils
{
public:
	static DLL_OrderStatus Get_OrderStatus_FromString(const std::string& status)
	{
		if (status == "OPEN")
			return DLL_OrderStatus::DLL_ORDER_STATUS_OPEN;
		else if (status == "CLOSED")
			return DLL_OrderStatus::DLL_ORDER_STATUS_CLOSED;
		else
			return DLL_OrderStatus::DLL_ORDER_STATUS_NONE;
	}
	static std::string Get_OrderStatus_String(DLL_OrderStatus status)
	{
		switch (status)
		{
		case DLL_OrderStatus::DLL_ORDER_STATUS_OPEN:
			return "OPEN";
		case DLL_OrderStatus::DLL_ORDER_STATUS_CLOSED:
			return "CLOSED";
		default:
			return "NONE";
		}
	}

};


class DLL_CoinsPairUtils {

public:
	static DLL_CoinsPair Get_CoinsPair_FromString(const std::string& pair)
	{
		if (pair == "XAUUSD") return DLL_CoinsPair::DLL_COINS_PAIR_XAUUSD;
		if (pair == "EURUSD") return DLL_CoinsPair::DLL_COINS_PAIR_EURUSD;
		return DLL_CoinsPair::DLL_COINS_PAIR_NONE;
	}
	static std::string Get_CoinsPair_String(DLL_CoinsPair pair)
	{
		switch (pair)
		{
		case DLL_CoinsPair::DLL_COINS_PAIR_XAUUSD:
			return "XAUUSD";
		case DLL_CoinsPair::DLL_COINS_PAIR_EURUSD:
			return "EURUSD";
		default:
			return "NONE";
		}
	}

};
