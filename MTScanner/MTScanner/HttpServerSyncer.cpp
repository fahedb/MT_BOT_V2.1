#include "HttpServerSyncer.h"

void HttpServerSyncer::UpdateCurrentTimeLine(BasePricesTable* table)
{
	ConfigrationsPolicy* policy = Configrations::getInstance()->Get_Policy();
	ExchangeData exChData;

	std::string body = exChData.Get_NextCandleStick_M1();
	Logger& logger = Logger::getInstance();
	Static_CurrentTimeLineRep* currTimeLineRep = Static_CurrentTimeLineRep::getInstance();

	while (body.length())
	{
		logger.WriteLog("Recieved a new bar by HTTP Client");
		JsonSelectionNode* selectionJ;

		try {
			nlohmann::json select = nlohmann::json::parse(body);
			selectionJ = new JsonSelectionNode(select);
		}
		catch (const std::exception& e) {
			logger.WriteLog("Error parsing JSON: " + std::string(e.what()), false, true, LogType::ERR);
			return;
		}
		table->Insert_NewCandleBody(selectionJ);
		ChangeNode* chn = table->GetNextChangeNode();
		currTimeLineRep->Set_NewBar(chn, true);
		body = exChData.Get_NextCandleStick_M1();
	}

	logger.WriteLog("No new Bars to recieve");
}

void HttpServerSyncer::UpdateConfigs()
{
	ExchangeData exh;
	std::pair<std::string, std::string> conf = exh.Get_ConfToUpdate();
	Configrations* configarions = Configrations::getInstance();

	while (conf.first.length())
	{
		configarions->UpdateConfigValue_DB(conf.first, conf.second);
		conf = exh.Get_ConfToUpdate();
	}
	configarions->RefreshPolicy(); // Refresh all policies 
}

void HttpServerSyncer::LoadAllConfigsToHttpCore()
{
	ExchangeData exh;
	std::vector<std::vector<std::string>> allConfigs = Configrations::getInstance()->Get_AllConfigRows_DB();
	for (int i = 0; i < allConfigs.size(); i++)
	{
		exh.Push_NewConfig(allConfigs.at(i));
	}
}
