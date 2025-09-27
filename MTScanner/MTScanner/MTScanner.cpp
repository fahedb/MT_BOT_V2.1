#include "StrategiesRunner.h"
#include "ProccessFlags.h"
#include "CONFIGRATIONS.H"

void DefualtProgramStart();
void LoadJson_Data_ToDBFile();


int main(int argc, char* argv[])
{
	ProccessFlags::InitProccessFlags(argc, argv);

	std::thread http_thread(ExchangeData::Start_DataExchange);
	if (ProccessFlags::FindSwitch("-dataLoad"))
	{
		LoadJson_Data_ToDBFile();
	}
	else {
		DefualtProgramStart();
	}
	http_thread.join();
}

void DefualtProgramStart()
{
	StrategiesRunner runner;
	runner.Run();
}



void LoadJson_Data_ToDBFile()
{
	std::string dbFilePath = Configrations::getInstance()->Get_DB_FileDir() + Configrations::getInstance()->Get_Config_Main(DBFILE_XAUUSD_M1_DATA_JSON);
	ExchangeData exChData;
	std::ofstream file(dbFilePath);
	Logger& logger = Logger::getInstance();
	long counter = 1;
	logger.WriteLog("Start reciveing data!", true);
	while (true)
	{
		std::string body = exChData.Get_NextCandleStick_M1_JSON_DB();
		if (body.length())
		{
			file << body << "\n";
			logger.WriteLog("Recieved a New Bar to Json File, Bar number: " + std::to_string(counter));
			counter++;
		}
	}
}
