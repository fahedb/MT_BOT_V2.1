// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "PatternGod.h"
#include <iostream>

int main()
{
    std::cout << "Hello World!\n";
	std::vector<ChangeNode*> historyTL; // Example history time line
	std::vector<ChangeNode*> currentTL; // Example current time line

	PatternGod patternSca(&historyTL, &currentTL);
}

void LoadHistoryData(std::vector<ChangeNode*>& historyTL)
{

}