#pragma once
#include <string>
#include "OrderType.h"
struct Deal
{
	OrderType type; 
	int ticket; // unique deal identifier
	long long openTime; // Format: YYYYMMDDHHMMSSMMM
	long long closeTime; // Format: YYYYMMDDHHMMSSMMM

	double lotSize; // in lots

	double openPrice; // in price units
	double closePrice; // in price units
	double profit; // in account currency
	int profitInPips; // in pips

	bool stopLossHit; // true if stop loss was hit
	bool takeProfitHit; // true if take profit was hit

	std::string symbol; // e.g., "XAUUSD"
};
