#pragma once
#include <string>
#include "OrderType.h"
#include "ClosureReason.h"

struct Deal
{
	OrderType type; 
	int ticket; // unique deal identifier
	long long openTime; // Format: YYYYMMDDHHMMSSMMM
	long long closeTime; // Format: YYYYMMDDHHMMSSMMM

	double lotSize; // in lots

	double openPrice; // in price units
	double closePrice; // in price units

	double stopLossPrice; // in price units
	double takeProfitPrice; // in price units

	double profit; // in account currency
	int profitInPips; // in pips

	int numOfBarsHeld = 0; // number of bars the deal was held open
	ClosureReason closeReason;

	std::string symbol; // e.g., "XAUUSD"
};
