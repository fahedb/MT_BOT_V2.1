#include <string>
#include <vector>
#include <fstream>
#include "LOGGER.H"

const std::string CONTAINER_FOLDER = "c:\\MTDB\\";

#pragma once
class CSVExportData
{
	std::string m_fileName;
	std::vector<std::string> m_dataTempContainer;
	std::ofstream* m_file;
public:
	CSVExportData(std::string fileName)
	{
		m_fileName = CONTAINER_FOLDER + fileName;
		m_file = new std::ofstream(m_fileName, std::ios::app);
	}
	~CSVExportData()
	{
		m_dataTempContainer.clear();
		m_file->close();
	}

	void InsertNewColumn(const std::string col)
	{
		m_dataTempContainer.push_back(col);
	}


	void InsertNewColumn(const std::vector<std::string> cols)
	{
		for (int i = 0; i < cols.size(); i++)
		{
			m_dataTempContainer.push_back(cols.at(i));
		}
	}

	void NewRow()
	{
		if (!m_file->is_open())
		{
			Logger::getInstance().WriteLog("Could not open csv file :" + m_fileName, false, true,LogType::ERR);
			return;
		}
		for (int i = 0; i < m_dataTempContainer.size(); i++) {
			(*m_file) << m_dataTempContainer.at(i) << ",";
		}
		(*m_file) << "\n";
		m_file->flush();
	}

	void DoneInsertingData()
	{
		m_file->close();
	}

};

