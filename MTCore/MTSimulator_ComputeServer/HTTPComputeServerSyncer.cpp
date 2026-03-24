#include "HTTPComputeServerSyncer.h"

void HTTPComputeServerSyncer::UpdateCurrentTimeLine(BasePricesTable* table, HTTPComputeServer* httpServer)
{

	std::string body = httpServer->Get_RequestHandler()->Get_NextCandleStick_M1_History();

	Static_CurrentTimeLineRep* currTimeLineRep = Static_CurrentTimeLineRep::getInstance();
	bool newBar = false;
	while (body.length())
	{
		newBar = true;
		std::cout << "Recieved a new bar by HTTP Client" << std::endl;;
		JsonSelectionNode* selectionJ;

		try {
			nlohmann::json select = nlohmann::json::parse(body);
			selectionJ = new JsonSelectionNode(select);
		}
		catch (const std::exception& e) {
			std::cout << "Error parsing JSON: " << std::string(e.what());
			return;
		}
		table->Insert_NewCandleBody(selectionJ);
		ChangeNode* chn = table->GetNextChangeNode();
		currTimeLineRep->Set_NewBar(chn, true);
		body = httpServer->Get_RequestHandler()->Get_NextCandleStick_M1_History();
	}

	if (newBar) // If we have a new bar, we need to update the current timeline polynom cache
	{
		int polysBuilt = PolynomTimeLineCacheManager::Build_PoylnomTimeLines(*currTimeLineRep->Get_CurrTimeLine()->Get_TimeLine(), CacheType::CURRENT);
		std::cout << "Built " + std::to_string(polysBuilt) << " polynomial timelines from the current timeline." << std::endl;
	}
	std::cout << "No new Bars to recieve" << std::endl;
}
