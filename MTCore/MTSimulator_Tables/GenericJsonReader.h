#pragma once
#include "BTables.h"
#include <future>

class GenericJsonReader : public BTables
{
	private:
		std::string m_filePath;
		std::vector<nlohmann::json> m_allJsons;
		int m_chunkSize = 0;
public:
	GenericJsonReader(const std::string& filePath) : BTables(filePath)
	{
		m_filePath = filePath;
	}
	void Load();
	std::vector<nlohmann::json>* GetAllJsons() { return &m_allJsons; }

	std::string GetDB_PhysicalFileName() override { return m_filePath; }
	std::string GetDB_PhysicalFileName_Json() override { return m_filePath; }

private:
	static void processChunk(int threadIndex, int chunkSize, GenericJsonReader* tableObj, std::vector<std::vector<nlohmann::json>>* reads2D);
	static void DataMerger(std::vector<std::vector<nlohmann::json>>* reads2D, std::vector<nlohmann::json>* allReads);
	void Set_DATA_DB_FileName(const std::string& fileName) { Set_DBFileName(fileName); }
	void Set_MultiThreadedRead() { Set_AsMultiThread_Read(); }
	int InitFileMapForRead();
	
};

