#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// OrderType type and utils ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

enum class OrderType
{
	LONG = 0,
	SHORT = 1,
	NONE = -1
};

class OrderTypeUtils
{
public:
	static OrderType Get_OrderType_FromString(const std::string& type)
	{
		if (type == "LONG")
			return OrderType::LONG;
		else if (type == "SHORT")
			return OrderType::SHORT;
		else
			return OrderType::NONE;
	}
	static std::string Get_OrderType_String(OrderType type)
	{
		switch (type)
		{
		case OrderType::LONG:
			return "LONG";
		case OrderType::SHORT:
			return "SHORT";
		default:
			return "NONE";
		}
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// OrderStatus type and utils ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

enum class OrderStatus
{
	OPEN = 0,
	CLOSED = 1,
	NONE = -1
};

class OrderStatusUtils
{
public:
	static OrderStatus Get_OrderStatus_FromString(const std::string& status)
	{
		if (status == "OPEN")
			return OrderStatus::OPEN;
		else if (status == "CLOSED")
			return OrderStatus::CLOSED;
		else
			return OrderStatus::NONE;
	}
	static std::string Get_OrderStatus_String(OrderStatus status)
	{
		switch (status)
		{
		case OrderStatus::OPEN:
			return "OPEN";
		case OrderStatus::CLOSED:
			return "CLOSED";
		default:
			return "NONE";
		}
	}

};