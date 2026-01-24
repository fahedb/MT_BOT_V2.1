#pragma once
#include <nlohmann/json.hpp>
#include "GenericBaseExporter.h"
#include <fstream>
#include "LOGGER.H"

class GenericJsonExporter : public GenericBaseExporter<nlohmann::json>
{
private:
	std::ofstream* m_outFile; // Open file in append mode

public:

	GenericJsonExporter(const std::string& exporterName, const std::string& dstName, int exportBatchSize = 1000)
		: GenericBaseExporter<nlohmann::json>(exporterName, dstName, exportBatchSize)
	{
		m_outFile = new std::ofstream(dstName, std::ios::out | std::ios::app);
	}
	~GenericJsonExporter();

protected:	
	virtual void ExportBatch(const std::vector<nlohmann::json>& batch) override;
};

