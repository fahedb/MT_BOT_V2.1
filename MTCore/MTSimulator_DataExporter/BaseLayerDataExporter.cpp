#include "BaseLayerDataExporter.h"
#include "LOGGER.H"
#include "LayersJsonKeys.h"
#include "ProccessFlags.h"
#include "Converts.h"

void BaseLayerDataExporter::AutoSubmitAction()
{
	m_autoSubmitCounter++;
	if (m_autoSubmit > 0 && m_autoSubmitCounter >= m_autoSubmit)
	{
		SubmitBackTestDataFile();
		m_autoSubmitCounter = 0;
	}
}

BaseLayerDataExporter::BaseLayerDataExporter(std::string name)
{
	std::string autoSubmitstr = ProccessFlags::GetSwitch("-autoSubmitBKTST");
	m_autoSubmit = autoSubmitstr.length() ? Converts::ToInt(autoSubmitstr) : 0;
	m_filePath = name;
}

void BaseLayerDataExporter::AddPolicyData(BasePolicy* policy)
{
	auto allPolicies = policy->Get_All();
	for (auto polc : allPolicies)
	{
		m_policiesData += polc.at(0) + "=" + polc.at(1);
		m_policiesData += "\n";
	}
}

void BaseLayerDataExporter::SubmitBackTestDataFile()
{
	if (GetLayerType() == LayerType::NONE)
	{
		Logger::getInstance().WriteLog("BaseLayerDataManager::SubmitBackTestDataFile - LayerType is NONE, cannot submit backtest data file", false, true, LogType::ERR);
		return;
	}

	if (!m_firstSubmitDone)
	{
		std::string netFileName = m_filePath.empty() ? DEFUALT_FILE_NAME : m_filePath;
		std::string fileExtension = LayerTypeList::GetFileExtention(GetLayerType());
		m_filePath = BASE_PATH_FOR_ALL_LAYERS_DATA_EXPORT + fileExtension + "\\" + netFileName + "." + fileExtension;
	}
	std::ofstream outFile;
	
	if (!m_firstSubmitDone)
	{
		outFile.open(m_filePath);
	}
	else
	{
		outFile.open(m_filePath, std::ios_base::app); // Append mode for subsequent submissions
	}

	if(!outFile.is_open())
	{
		Logger::getInstance().WriteLog("BaseLayerDataManager::SubmitBackTestDataFile - Could not open file: " + m_filePath, false, true, LogType::ERR);
		return;
	}

	if (!m_firstSubmitDone) { // If this is the first submit, write policies data
		outFile << PoliciesDataKey << "\n";
		outFile << m_policiesData << "\n\n";
		outFile << DataSummaryKey << "\n";
		m_firstSubmitDone = true; // Mark that the first submit is done
	}

	for (const auto& summary : m_dataSummary)
	{
		outFile << summary.dump() << "\n";
	}
	m_dataSummary.clear(); // Clear data summary after submission

}
