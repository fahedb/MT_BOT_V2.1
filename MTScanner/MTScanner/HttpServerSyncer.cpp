#include "HttpServerSyncer.h"
#include "PolynomTimeLineCache.h"

void HttpServerSyncer::Wait_ForNewBarM1() {
	ExchangeData().WaitFor_NewBar_M1(); // Wait for new M1 bar
	ExchangeData().Reset_NewBar_M1_Flag(); // Reset the flag after waiting
}

bool HttpServerSyncer::ShouldSubmitLayersExportersData()
{
	if(ExchangeData().Get_BKTST_SubmissionName().length())
		return true;
	return false;
}

void HttpServerSyncer::UpdateCurrentTimeLine(BasePricesTable* table)
{
	ExchangeData exChData;

	std::string body = exChData.Get_NextCandleStick_M1();
	Logger& logger = Logger::getInstance();
	Static_CurrentTimeLineRep* currTimeLineRep = Static_CurrentTimeLineRep::getInstance();
	bool newBar = false;
	while (body.length())
	{
		newBar = true;
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

	if (newBar) // If we have a new bar, we need to update the current timeline polynom cache
	{
		int polysBuilt = PolynomTimeLineCacheManager::Build_PoylnomTimeLines(*currTimeLineRep->Get_CurrTimeLine()->Get_TimeLine(), CacheType::CURRENT);
		logger.WriteLog("Built " + std::to_string(polysBuilt) + " polynomial timelines from the current timeline.", true, true, LogType::LOG);
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
		ConfigsTableStaticProxy::UpdateConfigValue_DB(conf.first, conf.second); // Update the config value in the database
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
