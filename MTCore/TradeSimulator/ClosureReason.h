#pragma once

enum class ClosureReason
{
	NONE = 0,
	STOP_LOSS_HIT = 1,
	TAKE_PROFIT_HIT = 2,
	FORCE_CLOSE = 3,
	STOP_OUT = 4
};
