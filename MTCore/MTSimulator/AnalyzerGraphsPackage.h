#include "GraphStructres.h"
#include <string>

#pragma once

class AnalyzerGraphContainer
{
private:
	GraphStructure m_struct;
	std::string m_pngPath; // Path to the PNG file of the graph
public:
	void Set_Structure(const GraphStructure& structure) { m_struct = structure; }
	void Set_PngPath(const std::string& path) { m_pngPath = path; }

	GraphStructure& Get_Structure() { return m_struct; }
	std::string Get_PngPath() const { return m_pngPath; }

	GraphStructure* Get_StructurePtr() { return &m_struct; }
	std::string* Get_PngPathPtr() { return &m_pngPath; }
};

class AnalyzerFullGraphsContainer
{
private:
	AnalyzerGraphContainer m_currentGraph; // Current Graph Container
	std::vector<AnalyzerGraphContainer> m_patternsGraphs; // Patterns Graphs

public:
	void Set_CurrentGraph(AnalyzerGraphContainer& graph) { m_currentGraph = graph; }
	void Add_PatternGraph(AnalyzerGraphContainer& graph) { m_patternsGraphs.push_back(graph); }

	AnalyzerGraphContainer& Get_CurrentGraph() { return m_currentGraph; }
	std::vector<AnalyzerGraphContainer>& Get_PatternsGraphs() { return m_patternsGraphs; }

	AnalyzerGraphContainer* Get_CurrentGraphPtr() { return &m_currentGraph; }
	std::vector<AnalyzerGraphContainer>* Get_PatternsGraphsPtr() { return &m_patternsGraphs; }

	AnalyzerGraphContainer& Get_PatternsGraph(int index) 
	{ 
		if (index < 0 || index >= m_patternsGraphs.size()) 
			throw std::out_of_range("Index out of range for patterns graphs");
		return m_patternsGraphs[index]; 
	}
	AnalyzerGraphContainer* Get_PatternsGraphPtr(int index) 
	{ 
		if (index < 0 || index >= m_patternsGraphs.size()) 
			throw std::out_of_range("Index out of range for patterns graphs");
		return &m_patternsGraphs[index]; 
	}
};

class AnalyzerGraphsPackage
{
private:
	bool m_isATrade = false; // Is a trade happened in the current time line
	std::string m_id; // Unique ID for the package
	AnalyzerFullGraphsContainer m_linearGraph; // Linear Graph Container
	AnalyzerFullGraphsContainer m_barsGraph; // Bars Graph Container

public:
	void Set_LinearGraphs(AnalyzerFullGraphsContainer& fullContainer) { m_linearGraph = fullContainer; }
	void Set_BarsGraphs(AnalyzerFullGraphsContainer& fullContainer) { m_barsGraph = fullContainer; }
	void Set_Id(const std::string& id) { m_id = id; }
	void Set_IsATrade(bool isTrade) { m_isATrade = isTrade; }

	std::string Get_Id() const { return m_id; }
	bool IsATrade() const { return m_isATrade; }
	AnalyzerFullGraphsContainer& Get_LinearGraphs() { return m_linearGraph; }
	AnalyzerFullGraphsContainer& Get_BarsGraphs() { return m_barsGraph; }

	AnalyzerFullGraphsContainer* Get_LinearGraphsPtr() { return &m_linearGraph; }
	AnalyzerFullGraphsContainer* Get_BarsGraphsPtr() { return &m_barsGraph; }

};

