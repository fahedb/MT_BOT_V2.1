#include "DBMap.h"
int DBMap::GetIdx_Of_TimePoint(std::string dateTime)
{
	auto pr = myMap.find(dateTime);
	if (pr != myMap.end())
	{
		return pr->second;
	}
	return -1;
}

