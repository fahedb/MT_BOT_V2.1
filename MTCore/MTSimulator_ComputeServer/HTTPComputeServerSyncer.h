#pragma once
#include "BasePricesTable.h"
#include "HTTPComputeServer.h"
#include "Static_CurrentTimeLineRep.h"
#include "PolynomTimeLineCache.h"
class HTTPComputeServerSyncer
{
public:
	static void UpdateCurrentTimeLine(BasePricesTable* table, HTTPComputeServer* httpServer);
};

