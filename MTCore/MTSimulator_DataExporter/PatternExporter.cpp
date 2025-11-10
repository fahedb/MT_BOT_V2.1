#include "PatternExporter.h"
#include "PROCESSDATA.H"

/// <PatternExporter>
/// PatternExporter class function, to export the found patterns to a diffrent methods, like simple png chart
/// <PatternExporter>

std::vector<std::string> PatternExporter::m_m1_patternsPaths;
std::vector<std::string> PatternExporter::m_m5_patternsPaths;
std::vector<std::string> PatternExporter::m_m15_patternsPaths;

void PatternExporter::Export_All_Pattens_ToPNG(std::vector<ChangeNode*>* currPattern, std::vector<std::vector<ChangeNode*>*>* similarPatterns)
{
	std::string containerFolder = Configrations::getInstance()->Get_DB_FileDir() + PNG_FOLDER_NAME + Generate_PNG_Dir_Name_ForMoment();
	if (!OpenA_PNG_DIR(containerFolder))
	{
		Logger::getInstance().WriteLog("Error In Opening a Dir for png moment: " + containerFolder);
		return;
	}
	Export_RelationalPatterns_EMA15_ToPNG(currPattern, similarPatterns, containerFolder);
	Export_RelationalPatterns_EMA5_ToPNG(currPattern, similarPatterns, containerFolder);
	Export_RelationalPatterns_EMA1_ToPNG(currPattern, similarPatterns, containerFolder);

	ExportDataLayerPolicy* exportPolicy = Configrations::getInstance()->Get_Policy<ExportDataLayerPolicy>(PolicyType::ExportDataLayerPolicy);

	if (exportPolicy->Export_Patterns_PNG_To_Telegram)
	{
		Logger::getInstance().LogPatternPngs(m_m15_patternsPaths, "M15", "L" + std::to_string(exportPolicy->EMA_len_for_Pattern_Export));
		Logger::getInstance().LogPatternPngs(m_m5_patternsPaths, "M5", "L" + std::to_string(exportPolicy->EMA_len_for_Pattern_Export));
		Logger::getInstance().LogPatternPngs(m_m1_patternsPaths, "M1", "L" + std::to_string(exportPolicy->EMA_len_for_Pattern_Export));
	}
	m_m15_patternsPaths.clear();
	m_m5_patternsPaths.clear();
	m_m1_patternsPaths.clear();

}

void PatternExporter::Export_RelationalPatterns_EMA1_ToPNG(std::vector<ChangeNode*>* currPattern, std::vector<std::vector<ChangeNode*>*>* similarPatterns, std::string containerFolder)
{
	if (!currPattern || !similarPatterns)
	{
		Logger::getInstance().WriteLog("Error: No Current&Similar TimeLines!!\nExport To PNG canceled");
	}
	GraphStructure currPatternPoints;
	std::vector<GraphStructure> similarPatternsPoints;
	currPatternPoints = Build_Points(currPattern, BarPeriod::M1, IndicatorType::EMA);
	similarPatternsPoints = Build_Points(similarPatterns, BarPeriod::M1, IndicatorType::EMA);

	containerFolder += "\\EMA_M1";
	if (!OpenA_PNG_DIR(containerFolder))
	{
		Logger::getInstance().WriteLog("Error In Opening a Dir for png moment: " + containerFolder);
		return;
	}
	std::string currentPNGFileName = containerFolder + "\\" + "EMA1_" + Generate_PNG_FileName(currPattern->at(0)->point_DateTime);
	m_m1_patternsPaths.push_back(currentPNGFileName);

	MTChart currentChart(currPatternPoints.points.at(0).first - currPatternPoints.points.at(0).second, static_cast<double>(currPatternPoints.points.size()));
	currentChart.DrawGraph(currPatternPoints, currentPNGFileName);

	for (int i = 0; i < similarPatternsPoints.size(); i++)
	{
		std::string historyPNGFileName = containerFolder + "\\" + "EMA1_" + Generate_PNG_FileName(similarPatterns->at(i)->at(0)->point_DateTime, true);
		m_m1_patternsPaths.push_back(historyPNGFileName);
		MTChart historyChart(similarPatternsPoints.at(i).points.at(0).first - similarPatternsPoints.at(i).points.at(0).second, static_cast<double>(similarPatternsPoints.at(i).points.size()));
		historyChart.DrawGraph(similarPatternsPoints.at(i), historyPNGFileName);
	}
}
void PatternExporter::Export_RelationalPatterns_EMA15_ToPNG(std::vector<ChangeNode*>* currPattern, std::vector<std::vector<ChangeNode*>*>* similarPatterns, std::string containerFolder)
{
	if (!currPattern || !similarPatterns)
	{
		Logger::getInstance().WriteLog("Error: No Current&Similar TimeLines!!\nExport To PNG canceled");
	}
	GraphStructure currPatternPoints;
	std::vector<GraphStructure> similarPatternsPoints;
	currPatternPoints = Build_Points(currPattern, BarPeriod::M15, IndicatorType::EMA);
	similarPatternsPoints = Build_Points(similarPatterns, BarPeriod::M15, IndicatorType::EMA);

	containerFolder += "\\EMA_M15";
	if (!OpenA_PNG_DIR(containerFolder))
	{
		Logger::getInstance().WriteLog("Error In Opening a Dir for png moment: " + containerFolder);
		return;
	}
	std::string currentPNGFileName = containerFolder + "\\" + "EMA15_" + Generate_PNG_FileName(currPattern->at(0)->point_DateTime);
	m_m15_patternsPaths.push_back(currentPNGFileName);
	
	MTChart currentChart(currPatternPoints.points.at(0).first - currPatternPoints.points.at(0).second, static_cast<double>(currPatternPoints.points.size()));
	currentChart.DrawGraph(currPatternPoints, currentPNGFileName);

	for (int i = 0; i < similarPatternsPoints.size(); i++)
	{
		std::string historyPNGFileName = containerFolder + "\\" + "EMA15_" + Generate_PNG_FileName(similarPatterns->at(i)->at(0)->point_DateTime, true);
		m_m15_patternsPaths.push_back(historyPNGFileName);

		MTChart historyChart(similarPatternsPoints.at(i).points.at(0).first - similarPatternsPoints.at(i).points.at(0).second, static_cast<double>(similarPatternsPoints.at(i).points.size()));
		historyChart.DrawGraph(similarPatternsPoints.at(i), historyPNGFileName);
	}
}

void PatternExporter::Export_RelationalPatterns_EMA5_ToPNG(std::vector<ChangeNode*>* currPattern, std::vector<std::vector<ChangeNode*>*>* similarPatterns, std::string containerFolder)
{
	if (!currPattern || !similarPatterns)
	{
		Logger::getInstance().WriteLog("Error: No Current&Similar TimeLines!!\nExport To PNG canceled");
	}
	GraphStructure currPatternPoints;
	std::vector<GraphStructure> similarPatternsPoints;
	currPatternPoints = Build_Points(currPattern, BarPeriod::M5, IndicatorType::EMA);
	similarPatternsPoints = Build_Points(similarPatterns, BarPeriod::M5, IndicatorType::EMA);

	containerFolder += "\\EMA_M5";
	if (!OpenA_PNG_DIR(containerFolder))
	{
		Logger::getInstance().WriteLog("Error In Opening a Dir for png moment: " + containerFolder);
		return;
	}
	std::string currentPNGFileName = containerFolder + "\\" + "EMA5_" + Generate_PNG_FileName(currPattern->at(0)->point_DateTime);
	m_m5_patternsPaths.push_back(currentPNGFileName);
	MTChart currentChart(currPatternPoints.points.at(0).first - currPatternPoints.points.at(0).second, static_cast<double>(currPatternPoints.points.size()));
	currentChart.DrawGraph(currPatternPoints, currentPNGFileName);

	for (int i = 0; i < similarPatternsPoints.size(); i++)
	{
		std::string historyPNGFileName = containerFolder + "\\" + "EMA5_" + Generate_PNG_FileName(similarPatterns->at(i)->at(0)->point_DateTime, true);
		m_m5_patternsPaths.push_back(historyPNGFileName);
		MTChart historyChart(similarPatternsPoints.at(i).points.at(0).first - similarPatternsPoints.at(i).points.at(0).second, static_cast<double>(similarPatternsPoints.at(i).points.size()));
		historyChart.DrawGraph(similarPatternsPoints.at(i), historyPNGFileName);
	}
}

bool PatternExporter::OpenA_PNG_DIR(std::string folderName)
{
	// Convert std::string to std::wstring because Windows API functions often use wide characters
	std::wstring wideFolderName(folderName.begin(), folderName.end());

	// Create the directory
	BOOL result = CreateDirectoryW(wideFolderName.c_str(), nullptr);
	bool res = false;
	if (result == 0) {
		// If creation fails, use GetLastError to get more info about the error
		DWORD error = GetLastError();
		if (error == ERROR_ALREADY_EXISTS) {
			res = true;
		}
		else {
			res = false;
		}
	}
	else {
		res = true;
	}
	return res;

}

std::string PatternExporter::Generate_PNG_Dir_Name_ForMoment()
{
	std::string fileName;
	fileName = ProcessData::GetFormattedCurrentDate('_') + "#" + ProcessData::GetFormattedCurrTime('_');
	static int counter = 0;
	static bool botton = false;
	if (botton)
	{
		botton = false;
		counter++;
	}
	else
		botton = true;
	if (!(counter % 100))
		counter = 0;
	fileName += "_" + std::to_string(counter);
	return fileName;
}

std::string PatternExporter::Generate_PNG_FileName(std::string formattedDateTime_Point, bool history)
{
	std::string fileName;
	fileName = formattedDateTime_Point + (history ? "_History.png" : "_Current.png");
	return fileName;
}

std::vector<GraphStructure> PatternExporter::Build_Points(std::vector<std::vector<ChangeNode*>*>* patternsNodes, BarPeriod mode, IndicatorType indT) // multi vector nodes
{
	std::vector<GraphStructure> pricesChanges;
	if (!patternsNodes || !(patternsNodes->size()))
		return pricesChanges;
	for (int i = 0; i < patternsNodes->size(); i++)
	{
		int splitter;
		GraphStructure graphPoints;
		Build_SinglePattern_PointsVector(patternsNodes->at(i), mode, indT, graphPoints);
		pricesChanges.push_back(graphPoints);
	}
	return pricesChanges;
}

GraphStructure PatternExporter::Build_Points(std::vector<ChangeNode*>* nodes, BarPeriod mode, IndicatorType indT) // single vector nodes
{
	GraphStructure pricesChange;
	if (!nodes || !(nodes->size()))
		return pricesChange;
	int splitter;
	Build_SinglePattern_PointsVector(nodes, mode, indT, pricesChange);
	return pricesChange;


}

void PatternExporter::Build_SinglePattern_PointsVector(std::vector<ChangeNode*>* nodes, BarPeriod mode, IndicatorType indT ,GraphStructure& pointsVec)
{
	// Need a config for indicatorLen
	PatternGodStatisticsPolicy* statisticsPolicy = Configrations::getInstance()->Get_Policy<PatternGodStatisticsPolicy>(PolicyType::PatternGodStatisticsPolicy);
	ScoringLayerPolicy* scoreConfPolicy = Configrations::getInstance()->Get_Policy<ScoringLayerPolicy>(PolicyType::ScoringLayerPolicy);
	ExportDataLayerPolicy* exportPolicy = Configrations::getInstance()->Get_Policy<ExportDataLayerPolicy>(PolicyType::ExportDataLayerPolicy);

	IndicatorLen indL = Indicators_Utils::Get_IndicatorLenEnum(exportPolicy->EMA_len_for_Pattern_Export);
	if (indL == IndicatorLen::NONE)
		indL = IndicatorLen::L20;

	std::pair<double, double> mxmn = Find_Max_Min_Values(nodes, mode, indT,indL);

	int minPoint = static_cast<int>(mxmn.second);
	int nodesCount = static_cast<int>(nodes->size());
	double counter = 0;
	double last_pointOnGraph = -1;
	pointsVec.points.push_back(mxmn);
	for (int i = 0; i < nodesCount; i++)
	{
		if (i == static_cast<int>(nodes->size()) - statisticsPolicy->Score_Future_Vision_Candles && scoreConfPolicy->Pattern_CurrentTimeLineLen < nodes->size())
		{
			pointsVec.splitter = counter;
		}
		double pointOnGraph = ProcessData::roundToDecimalPlaces(nodes->at(i)->Get_Indicator_Pointer(indT, indL)->Get_IndicatorValues_Pointer()->Get_Value(IndicatorValueType::Value, mode), (int)_Points::Points_XAUUSD);
		pointOnGraph -= minPoint;
		if (last_pointOnGraph == pointOnGraph)
		{
			continue;
		}
		last_pointOnGraph = pointOnGraph;
		pointsVec.points.push_back(std::pair<double, double>(counter, pointOnGraph));
		counter++;
	}
}


std::pair<double, double> PatternExporter::Find_Max_Min_Values(std::vector<ChangeNode*>* nodes, BarPeriod mode, IndicatorType indT,IndicatorLen indL)
{
	int nodesCount = static_cast<int>(nodes->size());
	double maxPoint = 0.0;
	int minPoint = 5000000;

	for (int i = 0; i < nodesCount; i++) // loop to find min\max values for cleaner chart /// TODO: Need to move to a new method
	{
		double pointOnGraph = ProcessData::roundToDecimalPlaces(nodes->at(i)->Get_Indicator_Pointer(indT, indL)->Get_IndicatorValues_Pointer()->Get_Value(IndicatorValueType::Value, mode), (int)_Points::Points_XAUUSD);
		if (maxPoint < static_cast<int>(pointOnGraph))
			maxPoint = static_cast<int>(pointOnGraph);
		if (minPoint > static_cast<int>(pointOnGraph))
			minPoint = static_cast<int>(pointOnGraph);
	}
	return std::pair<double, double>(maxPoint, minPoint);
}
