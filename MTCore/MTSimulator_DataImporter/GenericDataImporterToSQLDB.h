#pragma once

#include "GenericSQLTable.h"
#include "GenericJsonReader.h"

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

// NOTE: C++14 friendly. Comments in English by request.

class GenericJsonImporterToSQLDB
{
public:
    enum class ValueSource
    {
        RootPath,   // jsonData[...]
        VariantPath,// jsonData[variantBase][...]
        Constant    // constant literal
    };

    struct ColumnMapping
    {
        std::string dbColumn;                // Column name in SQL table
        ValueSource source = ValueSource::RootPath;

        // For RootPath/VariantPath: path segments inside JSON object
        std::vector<std::string> jsonPath;

        // For Constant:
        nlohmann::json constValue;

        // Optional: if true, string will be emitted as N'...' (good for NVARCHAR).
        bool unicodeString = true;
    };

    struct VariantRule
    {
        std::string name;                    // e.g. "General", "First", "Highest"
        std::vector<std::string> basePath;   // e.g. {"General_VotedPercentages"}
    };

public:
    GenericJsonImporterToSQLDB(const SQLHENV& henv,
        const SQLHDBC& hdbc,
        const std::string& tableName,
        const std::string& jsonFilePath);

    ~GenericJsonImporterToSQLDB() = default;

    // Common mappings (applied to every row produced from a JSON entry).
    void MapRootPath(const std::string& dbColumnName,
        const std::vector<std::string>& jsonKeyPath,
        bool unicodeString = true);

    // Variant mappings (read under variant base path).
    void MapVariantPath(const std::string& dbColumnName,
        const std::vector<std::string>& jsonKeyPath,
        bool unicodeString = true);

    // Constant mappings (applied to every row; can be overridden per-variant via SetVariantConstant).
    void MapConstant(const std::string& dbColumnName,
        const nlohmann::json& value,
        bool unicodeString = true);

    // Define variants. If no variants are added, importer uses a single implicit variant with empty basePath.
    void AddVariant(const std::string& variantName,
        const std::vector<std::string>& variantBasePath);

    // Per-variant constant override (e.g. VoteType = "General"/"First"...).
    void SetVariantConstant(const std::string& variantName,
        const std::string& dbColumnName,
        const nlohmann::json& value,
        bool unicodeString = true);

    bool ImportDataToSQLDB();

private:
    // Build "INSERT INTO table (col1, col2, ...)" template (columns are unique).
    void BuildInsertTemplateIfNeeded();

    // Build full insert for one produced row.
    std::string BuildInsertStatement(const std::vector<nlohmann::json>& orderedValues) const;

    // Produce 0..N rows from one JSON entry.
    bool InsertJsonToDB(const nlohmann::json& jsonData);

    // Helpers: JSON path traversal.
    static const nlohmann::json* TryGetByPath(const nlohmann::json& root, const std::vector<std::string>& path);

    // Convert json value to SQL literal string.
    static std::string JsonToSqlLiteral(const nlohmann::json& v, bool unicodeString);

    // Escape SQL string literal (single quotes).
    static std::string EscapeSqlString(const std::string& s);

private:
    std::unique_ptr<GenericSQLTable>  m_dataTable;
    std::unique_ptr<GenericJsonReader> m_jsonReader;

    std::vector<nlohmann::json>* m_allJsons = nullptr;

    std::vector<ColumnMapping> m_mappings; // all mappings (root/variant/constant)
    std::vector<VariantRule>   m_variants;

    // variantName -> list of constant overrides
    std::unordered_map<std::string, std::vector<ColumnMapping>> m_variantConstOverrides;

    // Insert template and ordered unique columns.
    bool m_templateBuilt = false;
    std::string m_insertPrefix;               // "INSERT INTO table (A,B,C)"
    std::vector<std::string> m_orderedColumns; // unique columns in deterministic order
};
