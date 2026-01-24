#include "GenericJsonExporter.h"

GenericJsonExporter::~GenericJsonExporter()
{
	if (m_outFile && m_outFile->is_open())
	{
		m_outFile->close();
		delete m_outFile;
		m_outFile = nullptr;
	}
}

void GenericJsonExporter::ExportBatch(const std::vector<nlohmann::json>& batch)
{
	if (!m_outFile->is_open())
	{
		Logger::getInstance().WriteLog("GenericJsonExporter::ExportBatch - Could not open file: " + m_dstName, false, true, LogType::ERR);
		return;
	}

	for (const auto& item : batch)
	{
		// Convert each nlohmann::json item to a string and export it
		std::string jsonString = item.dump();
		*m_outFile << jsonString << std::endl; // Write JSON string to file
	}
	m_outFile->flush();
}
