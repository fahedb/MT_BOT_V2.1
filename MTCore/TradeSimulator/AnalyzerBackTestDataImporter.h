#pragma once
#include "ProccessFlags.h"
#include "GenericDataImporterToSQLDB.h"
#include "MTExportTagsDef.h"
#include "Static_DBConnection.h"

class AnalyzerBackTestDataImporter
{
public:
	void BuildBackTestAnalyzeDB();
private:
	void InitAccountStateTable(GenericJsonImporterToSQLDB& importer);
	void InitClosedDealsTable(GenericJsonImporterToSQLDB& importer);
	void InitDirectionStatsTable(GenericJsonImporterToSQLDB& importer);
};

