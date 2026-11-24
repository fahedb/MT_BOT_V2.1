// ---------------------------------------------------------------------------
// CSV bar loading for the command-line tools.
//
// Deliberately the simplest possible input: the tools must be runnable from a
// plain export, without a live SQL Server or an MT5 terminal. Exporting from
// the project's existing tables is a one-line query, which means the new
// engine can be calibrated and backtested on real history before any of the
// live plumbing is rewired (Phase 6).
//
// Expected columns, with an optional header row:
//     timestamp,open,high,low,close[,volume]
// where timestamp is either milliseconds since the Unix epoch or
// "YYYY-MM-DD HH:MM:SS".
// ---------------------------------------------------------------------------
#pragma once

#include <charconv>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "mt/base/error.h"
#include "mt/data/bar.h"

namespace mtbot::app {

struct CsvLoadReport {
    std::size_t linesRead = 0;
    std::size_t barsAccepted = 0;
    std::size_t linesSkipped = 0;
    std::string firstSkipReason;
};

// Parses "YYYY-MM-DD HH:MM:SS" (or with 'T') as UTC milliseconds. Returns
// false if the shape does not match.
inline bool parseDateTimeMs(const std::string& text, mt::Timestamp& out) {
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    char sep = ' ';
    std::istringstream stream(text);
    stream >> year;
    if (stream.get() != '-') return false;
    stream >> month;
    if (stream.get() != '-') return false;
    stream >> day;
    sep = static_cast<char>(stream.get());
    if (sep != ' ' && sep != 'T') return false;
    stream >> hour;
    if (stream.get() != ':') return false;
    stream >> minute;
    if (stream.get() != ':') return false;
    stream >> second;
    if (year < 1970 || month < 1 || month > 12 || day < 1 || day > 31) return false;

    // Days from civil date, Howard Hinnant's algorithm. No timezone handling:
    // the input is treated as UTC, which is what the project's timestamps are.
    int y = year;
    const unsigned m = static_cast<unsigned>(month);
    const unsigned d = static_cast<unsigned>(day);
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    const long long days = static_cast<long long>(era) * 146097 +
                           static_cast<long long>(doe) - 719468;

    out = ((days * 24 + hour) * 60 + minute) * 60'000LL + second * 1000LL;
    return true;
}

inline mt::Result<mt::data::BarSeries> loadBarsFromCsv(const std::string& path,
                                                       const std::string& symbol,
                                                       CsvLoadReport& report) {
    std::ifstream in(path);
    if (!in) {
        return mt::ioFailure("loadBarsFromCsv: cannot open " + path);
    }

    mt::data::BarSeries bars(symbol);
    std::string line;
    bool firstLine = true;

    while (std::getline(in, line)) {
        ++report.linesRead;
        if (line.empty()) continue;

        // Split on comma, semicolon or tab, so common exports work unchanged.
        std::vector<std::string> fields;
        std::string field;
        for (const char c : line) {
            if (c == ',' || c == ';' || c == '\t') {
                fields.push_back(field);
                field.clear();
            } else if (c != '\r') {
                field.push_back(c);
            }
        }
        fields.push_back(field);

        if (fields.size() < 5) {
            ++report.linesSkipped;
            if (report.firstSkipReason.empty()) {
                report.firstSkipReason = "fewer than 5 columns on line " +
                                         std::to_string(report.linesRead);
            }
            continue;
        }

        mt::data::Bar bar;
        bool parsed = true;

        // Timestamp: milliseconds, or a date-time string.
        {
            const std::string& ts = fields[0];
            char* end = nullptr;
            const long long asNumber = std::strtoll(ts.c_str(), &end, 10);
            if (end != nullptr && *end == '\0' && asNumber > 0) {
                bar.ts = asNumber;
            } else if (!parseDateTimeMs(ts, bar.ts)) {
                parsed = false;
            }
        }

        const auto toDouble = [&parsed](const std::string& text, double& out) {
            char* end = nullptr;
            out = std::strtod(text.c_str(), &end);
            if (end == text.c_str()) {
                parsed = false;
            }
        };
        toDouble(fields[1], bar.open);
        toDouble(fields[2], bar.high);
        toDouble(fields[3], bar.low);
        toDouble(fields[4], bar.close);
        if (fields.size() > 5) {
            toDouble(fields[5], bar.volume);
        }

        if (!parsed) {
            if (firstLine) {
                firstLine = false;
                continue;  // almost certainly a header row
            }
            ++report.linesSkipped;
            if (report.firstSkipReason.empty()) {
                report.firstSkipReason =
                    "unparseable numbers on line " + std::to_string(report.linesRead);
            }
            continue;
        }
        firstLine = false;

        // Rejected bars are counted and reported, never silently dropped: a
        // feed that produces thousands of incoherent bars is something the
        // operator needs to know about, not something to tolerate.
        if (const mt::Status status = bars.append(bar); !status.ok()) {
            ++report.linesSkipped;
            if (report.firstSkipReason.empty()) {
                report.firstSkipReason = status.error().describe() + " (line " +
                                         std::to_string(report.linesRead) + ")";
            }
            continue;
        }
        ++report.barsAccepted;
    }

    if (bars.empty()) {
        return mt::dataCorrupt("loadBarsFromCsv: no usable bars in " + path +
                               (report.firstSkipReason.empty()
                                    ? ""
                                    : ". First problem: " + report.firstSkipReason));
    }
    return bars;
}

}  // namespace mtbot::app
