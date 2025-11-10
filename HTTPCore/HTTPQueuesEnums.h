#pragma once
#include <vector>

enum class HTTPQueueType
{
	NONE = -1,
	NEW_BKTST_XAUUSD_M1_DATA,
	SUBMIT_BKTST_XAUUSD_M1_RESULTS,


	COUNT // Total number of queue types -- this should be updated if more queue types are added
};

struct HTTPQueueTypeList { std::vector<HTTPQueueType> types = { HTTPQueueType::NEW_BKTST_XAUUSD_M1_DATA,HTTPQueueType::SUBMIT_BKTST_XAUUSD_M1_RESULTS }; };

enum class HTTPStackType
{
		NONE = -1,

		COUNT // Total number of stack types -- this should be updated if more stack types are added
};

struct HTTPStackTypeList { std::vector<HTTPStackType> types = { /* Should initialize types here */}; };