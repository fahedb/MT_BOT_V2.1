#include "ProccessFlags.h"
#include "CONFIGRATIONS.H"
#include "DataBaseConnection.h"
#include "ScoreResultsCachePolynom.h"

#include "PatternGodScanner.h"
void DefualtProgramStart();
void LoadJson_Data_ToDBFile();


int main(int argc, char* argv[])
{
	ScoreResultsCachePolynom* scoreResultsCache = ScoreResultsCachePolynom::getInstance();
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

void Init()
{
	std::string connStr = Configrations::getInstance()->Get_ConnectionString();
	DataBaseConnection::Set_ConnectionString(connStr);
}


void DefualtProgramStart()
{
	Init();
	PatternGodScanner scanner;
	scanner.Run();
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


