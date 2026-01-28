#include "GenericDataImporterToSQLDB.h"
#include "Logger.h"

#include <sstream>
#include <iomanip>

GenericJsonImporterToSQLDB::GenericJsonImporterToSQLDB(const SQLHENV& henv,
    const SQLHDBC& hdbc,
    const std::string& tableName,
    const std::string& jsonFilePath)
{
    m_dataTable = std::unique_ptr<GenericSQLTable>(new GenericSQLTable(henv, hdbc, tableName));
    m_jsonReader = std::unique_ptr<GenericJsonReader>(new GenericJsonReader(jsonFilePath));
    m_jsonReader->Load();
    m_allJsons = m_jsonReader->GetAllJsons();
}

void GenericJsonImporterToSQLDB::MapRootPath(const std::string& dbColumnName,
    const std::vector<std::string>& jsonKeyPath,
    bool unicodeString)
{
    ColumnMapping m;
    m.dbColumn = dbColumnName;
    m.source = ValueSource::RootPath;
    m.jsonPath = jsonKeyPath;
    m.unicodeString = unicodeString;
    m_mappings.push_back(std::move(m));
    m_templateBuilt = false;
}

void GenericJsonImporterToSQLDB::MapVariantPath(const std::string& dbColumnName,
    const std::vector<std::string>& jsonKeyPath,
    bool unicodeString)
{
    ColumnMapping m;
    m.dbColumn = dbColumnName;
    m.source = ValueSource::VariantPath;
    m.jsonPath = jsonKeyPath;
    m.unicodeString = unicodeString;
    m_mappings.push_back(std::move(m));
    m_templateBuilt = false;
}

void GenericJsonImporterToSQLDB::MapConstant(const std::string& dbColumnName,
    const nlohmann::json& value,
    bool unicodeString)
{
    ColumnMapping m;
    m.dbColumn = dbColumnName;
    m.source = ValueSource::Constant;
    m.constValue = value;
    m.unicodeString = unicodeString;
    m_mappings.push_back(std::move(m));
    m_templateBuilt = false;
}

void GenericJsonImporterToSQLDB::AddVariant(const std::string& variantName,
    const std::vector<std::string>& variantBasePath)
{
    VariantRule v;
    v.name = variantName;
    v.basePath = variantBasePath;
    m_variants.push_back(std::move(v));
}

void GenericJsonImporterToSQLDB::SetVariantConstant(const std::string& variantName,
    const std::string& dbColumnName,
    const nlohmann::json& value,
    bool unicodeString)
{
    ColumnMapping m;
    m.dbColumn = dbColumnName;
    m.source = ValueSource::Constant;
    m.constValue = value;
    m.unicodeString = unicodeString;
    m_variantConstOverrides[variantName].push_back(std::move(m));
    m_templateBuilt = false;
}

bool GenericJsonImporterToSQLDB::ImportDataToSQLDB()
{
    if (!m_allJsons)
    {
        Logger::getInstance().WriteLog("Importer - JSON list is null", false, true, LogType::ERR);
        return false;
    }

    BuildInsertTemplateIfNeeded();

    for (const auto& jsonData : *m_allJsons)
    {
        if (!InsertJsonToDB(jsonData))
            return false;
    }
    return true;
}

void GenericJsonImporterToSQLDB::BuildInsertTemplateIfNeeded()
{
    if (m_templateBuilt)
        return;

    // If no variants were defined, create a single implicit variant.
    if (m_variants.empty())
    {
        VariantRule v;
        v.name = ""; // implicit
        v.basePath.clear();
        m_variants.push_back(std::move(v));
    }

    // Build unique columns list in insertion order.
    std::unordered_map<std::string, bool> seen;
    m_orderedColumns.clear();

    auto addCol = [&](const std::string& col)
        {
            if (seen.find(col) != seen.end())
                return;
            seen[col] = true;
            m_orderedColumns.push_back(col);
        };

    for (const auto& m : m_mappings)
        addCol(m.dbColumn);

    for (const auto& kv : m_variantConstOverrides)
        for (const auto& m : kv.second)
            addCol(m.dbColumn);

    std::ostringstream oss;
    oss << "INSERT INTO " << m_dataTable->GetDB_PhysicalFileName() << " (";
    for (size_t i = 0; i < m_orderedColumns.size(); ++i)
    {
        oss << m_orderedColumns[i];
        if (i + 1 < m_orderedColumns.size())
            oss << ", ";
    }
    oss << ")";

    m_insertPrefix = oss.str();
    m_templateBuilt = true;
}

std::string GenericJsonImporterToSQLDB::BuildInsertStatement(const std::vector<nlohmann::json>& orderedValues) const
{
    std::ostringstream oss;
    oss << m_insertPrefix << " VALUES (";

    for (size_t i = 0; i < orderedValues.size(); ++i)
    {
        // We don't know per-column unicode flag here, so we store it in json object wrapper earlier.
        // Strategy: orderedValues[i] is a json object { "v": <value>, "u": <bool> } for strings,
        // or a raw json for non-strings.
        if (orderedValues[i].is_object() && orderedValues[i].contains("v") && orderedValues[i].contains("u"))
        {
            const auto& v = orderedValues[i]["v"];
            const bool u = orderedValues[i]["u"].get<bool>();
            oss << JsonToSqlLiteral(v, u);
        }
        else
        {
            oss << JsonToSqlLiteral(orderedValues[i], true);
        }

        if (i + 1 < orderedValues.size())
            oss << ", ";
    }

    oss << ");";
    return oss.str();
}

bool GenericJsonImporterToSQLDB::InsertJsonToDB(const nlohmann::json& jsonData)
{
    // Produce a row per variant.
    for (const auto& variant : m_variants)
    {
        // Prepare value lookup: columnName -> value wrapper
        std::unordered_map<std::string, nlohmann::json> colValues;

        // 1) Apply base mappings
        for (const auto& m : m_mappings)
        {
            if (m.source == ValueSource::Constant)
            {
                // Store with unicode flag wrapper for strings
                if (m.constValue.is_string())
                    colValues[m.dbColumn] = nlohmann::json{ {"v", m.constValue}, {"u", m.unicodeString} };
                else
                    colValues[m.dbColumn] = m.constValue;
                continue;
            }

            if (m.source == ValueSource::RootPath)
            {
                const nlohmann::json* p = TryGetByPath(jsonData, m.jsonPath);
                if (!p)
                    colValues[m.dbColumn] = nullptr;
                else if (p->is_string())
                    colValues[m.dbColumn] = nlohmann::json{ {"v", *p}, {"u", m.unicodeString} };
                else
                    colValues[m.dbColumn] = *p;
                continue;
            }

            if (m.source == ValueSource::VariantPath)
            {
                const nlohmann::json* base = TryGetByPath(jsonData, variant.basePath);
                if (!base)
                {
                    colValues[m.dbColumn] = nullptr;
                    continue;
                }
                const nlohmann::json* p = TryGetByPath(*base, m.jsonPath);
                if (!p)
                    colValues[m.dbColumn] = nullptr;
                else if (p->is_string())
                    colValues[m.dbColumn] = nlohmann::json{ {"v", *p}, {"u", m.unicodeString} };
                else
                    colValues[m.dbColumn] = *p;
                continue;
            }
        }

        // 2) Apply per-variant constant overrides (e.g. VoteType)
        auto itOv = m_variantConstOverrides.find(variant.name);
        if (itOv != m_variantConstOverrides.end())
        {
            for (const auto& ov : itOv->second)
            {
                if (ov.constValue.is_string())
                    colValues[ov.dbColumn] = nlohmann::json{ {"v", ov.constValue}, {"u", ov.unicodeString} };
                else
                    colValues[ov.dbColumn] = ov.constValue;
            }
        }

        // 3) Build ordered values aligned to m_orderedColumns
        std::vector<nlohmann::json> ordered;
        ordered.reserve(m_orderedColumns.size());
        for (const auto& col : m_orderedColumns)
        {
            auto it = colValues.find(col);
            if (it == colValues.end())
                ordered.push_back(nullptr);
            else
                ordered.push_back(it->second);
        }

        // 4) Insert
        const std::string sql = BuildInsertStatement(ordered);
        m_dataTable->InsertRecord(sql);
    }

    return true;
}

const nlohmann::json* GenericJsonImporterToSQLDB::TryGetByPath(const nlohmann::json& root,
    const std::vector<std::string>& path)
{
    const nlohmann::json* cur = &root;
    for (const auto& key : path)
    {
        if (!cur)
            return nullptr;

        if (!cur->is_object())
            return nullptr;

        auto it = cur->find(key);
        if (it == cur->end())
            return nullptr;

        cur = &(*it);
    }
    return cur;
}

std::string GenericJsonImporterToSQLDB::EscapeSqlString(const std::string& s)
{
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s)
    {
        if (c == '\'')
            out += "''";
        else
            out += c;
    }
    return out;
}

std::string GenericJsonImporterToSQLDB::JsonToSqlLiteral(const nlohmann::json& v, bool unicodeString)
{
    if (v.is_null())
        return "NULL";

    if (v.is_boolean())
        return v.get<bool>() ? "1" : "0";

    if (v.is_number_integer())
        return std::to_string(v.get<long long>());

    if (v.is_number_unsigned())
        return std::to_string(v.get<unsigned long long>());

    if (v.is_number_float())
    {
        std::ostringstream oss;
        oss << std::setprecision(15) << v.get<double>();
        return oss.str();
    }

    if (v.is_string())
    {
        const std::string s = EscapeSqlString(v.get<std::string>());
        if (unicodeString)
            return "N'" + s + "'";
        return "'" + s + "'";
    }

    // For arrays/objects: store JSON string.
    const std::string s = EscapeSqlString(v.dump());
    return "N'" + s + "'";
}
