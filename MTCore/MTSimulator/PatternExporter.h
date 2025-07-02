#pragma once
#include <string>
#include <vector>
#include "ChangeData.h"
#include "MTChart.h"
#include "PROCESSDATA.H"
#include "Configrations.h"
#include "Logger.h"
#include "GraphStructres.h"

#define PNG_FOLDER_NAME "PatternsPNG\\"


/// <PatternExporter>
/// PatternExporter class function, to export the found patterns to a diffrent methods, like simple png chart
/// <PatternExporter>

class PatternExporter
{

private:
	static std::vector<std::string> m_m1_patternsPaths;
	static std::vector<std::string> m_m5_patternsPaths;
	static std::vector<std::string> m_m15_patternsPaths;

public:
	static void Export_All_Pattens_ToPNG(std::vector<ChangeNode*>* currPattern, std::vector<std::vector<ChangeNode*>*>* similarPatterns);
	static void Export_RelationalPatterns_EMA15_ToPNG(std::vector<ChangeNode*>* currPattern, std::vector<std::vector<ChangeNode*>*>* similarPatterns, std::string containerFolder);
	static void Export_RelationalPatterns_EMA5_ToPNG(std::vector<ChangeNode*>* currPattern, std::vector<std::vector<ChangeNode*>*>* similarPatterns, std::string containerFolder);
	static void Export_RelationalPatterns_EMA1_ToPNG(std::vector<ChangeNode*>* currPattern, std::vector<std::vector<ChangeNode*>*>* similarPatterns, std::string containerFolder);

private:
	static bool OpenA_PNG_DIR(std::string folderName);
	static std::string Generate_PNG_Dir_Name_ForMoment();

	static std::string Generate_PNG_FileName(std::string formattedDateTime_Point, bool history = false /*Defualt value is to generate name for current Pattern*/);
	static GraphStructure Build_Points(std::vector<ChangeNode*>* nodes, BarPeriod mode, IndicatorType indT);
	static std::vector<GraphStructure> Build_Points(std::vector<std::vector<ChangeNode*>*>* patternsNodes, BarPeriod mode, IndicatorType indT);

	static void Build_SinglePattern_PointsVector(std::vector<ChangeNode*>* nodes, BarPeriod mode, IndicatorType indT, GraphStructure& pointsVec);
	static double GetValueByMode(ChangeNode* chN, BarPeriod mode, IndicatorType indT,IndicatorLen indL);
	static std::pair<double, double> Find_Max_Min_Values(std::vector<ChangeNode*>* nodes, BarPeriod mode, IndicatorType indT,IndicatorLen indL);
};

