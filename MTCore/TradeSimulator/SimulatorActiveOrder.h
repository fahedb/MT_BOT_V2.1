#pragma once
#include <string>
#include "OrderType.h"

struct ActiveOrderInfo
{
	int orderId;            // Unique identifier for the order
	double lotSize;        // Size of the order in lots
	double openPrice;      // Price at which the order was opened
	long long openTime;    // Time when the order was opened (format: YYYYMMDDHHMMSSMMM)
	int slPips;            // Stop loss in pips
	int tpPips;            // Take profit in pips
	bool isActive;      // Indicates if the order is currently active
	OrderType type; // Type of the order (LONG or SHORT)
	std::string symbol; // Trading symbol, e.g., "XAUUSD"
	int numOfBarsHeld = 0; // Number of bars the order has been held open
};