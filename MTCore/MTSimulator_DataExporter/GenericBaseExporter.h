#pragma once
#include <vector>
#include <string>

template<typename T>
class GenericBaseExporter
{
protected:
	std::string m_exporterName;
	std::string m_dstName;

	std::vector<T> m_dataItems;
	int m_exportBatchSize;
public:
	GenericBaseExporter(const std::string& exporterName, const std::string& dstName, int exportBatchSize = 1000)
		: m_exporterName(exporterName), m_dstName(dstName), m_exportBatchSize(exportBatchSize)
	{
	}

	virtual ~GenericBaseExporter() = default;
	virtual void AddDataItem(const T& data);
protected:
	virtual void ExportBatch(const std::vector<T>& batch) = 0; // Pure virtual function to be implemented by derived classes
};

template<typename T>
void GenericBaseExporter<T>::AddDataItem(const T& data)
{
	m_dataItems.push_back(data);
	if (m_dataItems.size() >= static_cast<size_t>(m_exportBatchSize))
	{
		ExportBatch(m_dataItems);
		m_dataItems.clear();
	}
}
