#include "BaseLayerDataExporter.h"
#include "LOGGER.H"

BaseLayerDataExporter::BaseLayerDataExporter(std::string name)
{
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
	std::string netFileName = m_filePath.empty() ? DEFUALT_FILE_NAME : m_filePath;
	std::string fileExtension = LayerTypeList::GetFileExtention(GetLayerType());
	m_filePath = BASE_PATH_FOR_ALL_LAYERS_DATA_EXPORT + fileExtension + "\\" + netFileName + "." + fileExtension;

	std::ofstream outFile(m_filePath);
	if(!outFile.is_open())
	{
		Logger::getInstance().WriteLog("BaseLayerDataManager::SubmitBackTestDataFile - Could not open file: " + m_filePath, false, true, LogType::ERR);
		return;
	}

	outFile << "Policies Data:\n";
	outFile << m_policiesData << "\n\n";
	outFile << "Data Summery:\n";
	for (const auto& summery : m_dataSummery)
	{
		outFile << summery.dump() << "\n";
	}

}
