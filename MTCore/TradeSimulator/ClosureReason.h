#pragma once

enum class ClosureReason
{
	NONE = 0,
	STOP_LOSS_HIT = 1,
	TAKE_PROFIT_HIT = 2,
	FORCE_CLOSE = 3,
	STOP_OUT = 4
};

class ClosureReasonUtils
{
public:
	static std::string ToString(ClosureReason reason)
	{
		switch (reason)
		{
		case ClosureReason::NONE:
			return "NONE";
		case ClosureReason::STOP_LOSS_HIT:
			return "STOP_LOSS_HIT";
		case ClosureReason::TAKE_PROFIT_HIT:
			return "TAKE_PROFIT_HIT";
		case ClosureReason::FORCE_CLOSE:
			return "FORCE_CLOSE";
		case ClosureReason::STOP_OUT:
			return "STOP_OUT";
		default:
			return "UNKNOWN";
		}
	}

	static ClosureReason FromString(const std::string& str)
	{
		if (str == "NONE")
			return ClosureReason::NONE;
		else if (str == "STOP_LOSS_HIT")
			return ClosureReason::STOP_LOSS_HIT;
		else if (str == "TAKE_PROFIT_HIT")
			return ClosureReason::TAKE_PROFIT_HIT;
		else if (str == "FORCE_CLOSE")
			return ClosureReason::FORCE_CLOSE;
		else if (str == "STOP_OUT")
			return ClosureReason::STOP_OUT;
		else
			return ClosureReason::NONE; // Default to NONE for unknown strings
	}
};