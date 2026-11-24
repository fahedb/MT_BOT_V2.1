// ---------------------------------------------------------------------------
// Logging as an interface, not a singleton.
//
// The pre-refactor code reaches Logger::getInstance() from inside the maths
// (finding E7), which is why none of it could be unit-tested and why
// PolynomBuilder could not even be compiled in isolation. Nothing in libs/
// may depend on a global: a component that logs takes an ILogger&.
//
// Pure functions should not log at all. If a function needs to explain a
// failure, it returns an Error (mt/base/error.h).
// ---------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace mt {

enum class LogLevel { Trace = 0, Debug, Info, Warn, Error };

[[nodiscard]] constexpr std::string_view toString(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "INFO";
}

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void log(LogLevel level, std::string_view message) = 0;

    void trace(std::string_view m) { log(LogLevel::Trace, m); }
    void debug(std::string_view m) { log(LogLevel::Debug, m); }
    void info(std::string_view m)  { log(LogLevel::Info, m); }
    void warn(std::string_view m)  { log(LogLevel::Warn, m); }
    void error(std::string_view m) { log(LogLevel::Error, m); }
};

// Discards everything. The default for tests and for pure computation paths.
class NullLogger final : public ILogger {
public:
    void log(LogLevel, std::string_view) override {}

    static NullLogger& instance() {
        static NullLogger logger;
        return logger;
    }
};

// Writes to stderr. Thread-safe, unlike the std::cout calls scattered through
// the legacy worker threads.
class ConsoleLogger final : public ILogger {
public:
    explicit ConsoleLogger(LogLevel minLevel = LogLevel::Info) : m_minLevel(minLevel) {}

    void log(LogLevel level, std::string_view message) override {
        if (level < m_minLevel) return;
        const std::lock_guard<std::mutex> lock(m_mutex);
        std::cerr << '[' << toString(level) << "] " << message << '\n';
    }

    void setMinLevel(LogLevel level) { m_minLevel = level; }

private:
    LogLevel m_minLevel;
    std::mutex m_mutex;
};

// Keeps records in memory so a test can assert on what was logged.
class CapturingLogger final : public ILogger {
public:
    struct Record {
        LogLevel level;
        std::string message;
    };

    void log(LogLevel level, std::string_view message) override {
        const std::lock_guard<std::mutex> lock(m_mutex);
        m_records.push_back({level, std::string(message)});
    }

    [[nodiscard]] std::vector<Record> records() const {
        const std::lock_guard<std::mutex> lock(m_mutex);
        return m_records;
    }

    [[nodiscard]] std::size_t count(LogLevel level) const {
        const std::lock_guard<std::mutex> lock(m_mutex);
        std::size_t n = 0;
        for (const auto& r : m_records) {
            if (r.level == level) ++n;
        }
        return n;
    }

    void clear() {
        const std::lock_guard<std::mutex> lock(m_mutex);
        m_records.clear();
    }

private:
    mutable std::mutex m_mutex;
    std::vector<Record> m_records;
};

}  // namespace mt
