#include "BackTestingTimer.h"
#include "Static_CurrentTimeLineRep.h"


void BackTestingTimer::SendFirstBatchOfCurrTimeLineDataToScanner()
{
	Static_CurrentTimeLineRep* currTimeLineRep = Static_CurrentTimeLineRep::getInstance();
	ExchangeData dataExch;
	size_t initialBatchSize = CONST_FIRST_BATCH_SIZE; // Number of bars to send in the initial batch
	for (size_t i = 0; i < initialBatchSize && i < m_backTestData->size(); i++)
	{
		currTimeLineRep->Set_NewBar(m_backTestData->at(i), true);
	}
	dataExch.Notify_NewBar_M1(); // Notify the scanner of the new bar
}

void BackTestingTimer::InjectDataToConsoleScreen(int currIdx, int fullDataNum, double accountBalance)
{

	// This function should show a progress bar in the console screen
	float progress = static_cast<float>(currIdx) / static_cast<float>(fullDataNum);
	int barWidth = 50; // Width of the progress bar
	std::cout << "[";
	for (int i = 0; i < barWidth; ++i) {
		if (i < static_cast<int>(barWidth * progress))
			std::cout << "=";
		else
			std::cout << " ";
	}
	std::cout << "] " << static_cast<int>(progress * 100.0) << "% ";
	std::cout << " | Account Balance: $" << std::to_string(accountBalance) << "\r";
	std::cout.flush();
}
