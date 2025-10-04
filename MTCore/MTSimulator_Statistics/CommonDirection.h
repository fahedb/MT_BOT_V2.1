#pragma once

enum class CommonDirection
{
	UP = 0,
	DOWN = 1,
	BLUR = 3,
	NONE = 4 // Added NONE to handle cases where direction is not defined
};

class CommonDirection_Utils
{
public:

	static std::string GetCommonDirectionStr(CommonDirection dir)
	{
		switch (dir)
		{
		case CommonDirection::UP:
			return "UP";
		case CommonDirection::DOWN:
			return "DOWN";
		case CommonDirection::BLUR:
			return "BLUR";
		case CommonDirection::NONE:
			return "NONE";

		default:
			return "NONE";
		}

		return "NONE";
	}
	static CommonDirection GetCommonDirectionEnum(std::string dir)
	{
		if (dir == "UP")
			return CommonDirection::UP;
		else if (dir == "DOWN")
			return CommonDirection::DOWN;
		else if (dir == "BLUR")
			return CommonDirection::BLUR;
		else
			return CommonDirection::NONE;
	}

};

