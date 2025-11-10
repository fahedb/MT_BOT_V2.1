#pragma once
#include "nlohmann/json.hpp"
#include <vector>
#include "Policies.h"
#include "LayersEnums.h"

#define BASE_PATH_FOR_ALL_LAYERS_DATA_EXPORT "C:\\MTDB\\BackTests\\"
#define DEFUALT_FILE_NAME "MyLayerFile"

class BaseLayerDataExporter
{
protected:
	std::string m_policiesData;
	std::vector<nlohmann::json> m_dataSummery;
	std::string m_filePath;
	virtual LayerType GetLayerType() const = 0;
public:
	BaseLayerDataExporter(std::string name = "");
	virtual void AddPolicyData(BasePolicy* policy);
	virtual void SubmitBackTestDataFile();
};

