// ---------------------------------------------------------------------------
// Bar and BarSeries — OHLC data with timestamp lookup.
//
// Replaces ChangeNode plus DBMap. Differences that matter:
//
//  * value semantics. The legacy code keeps std::vector<ChangeNode*> pointing
//    into a separate std::vector<ChangeNode> memory pool that can reallocate
//    (finding E8); here the bars are owned in one vector and referred to by
//    index.
//  * intrabar high and low are first-class, because the first-touch label in
//    mt_labels needs them. The legacy direction logic summed a precomputed
//    Pip_Change per bar and so could not tell which side was touched first
//    (finding S9).
//  * timestamp lookup is on the container, not a parallel singleton map.
// ---------------------------------------------------------------------------
#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "mt/base/clock.h"
#include "mt/base/error.h"

namespace mt::data {

struct Bar {
    Timestamp ts = 0;  // milliseconds since the Unix epoch, bar open time
    double open = 0.0;
    double high = 0.0;
    double low = 0.0;
    double close = 0.0;
    double volume = 0.0;

    [[nodiscard]] double range() const noexcept { return high - low; }
    [[nodiscard]] double body() const noexcept { return close - open; }
    [[nodiscard]] bool isUp() const noexcept { return close > open; }

    // Self-consistency. Feeds that interleave sources, or a mid-bar snapshot,
    // can produce bars where this does not hold; better to reject them at the
    // boundary than to normalise a broken high/low into a pattern.
    [[nodiscard]] bool isCoherent() const noexcept {
        return high >= low && high >= open && high >= close && low <= open && low <= close;
    }
};

class BarSeries {
public:
    BarSeries() = default;
    explicit BarSeries(std::string symbol) : m_symbol(std::move(symbol)) {}

    [[nodiscard]] const std::string& symbol() const noexcept { return m_symbol; }
    void setSymbol(std::string symbol) { m_symbol = std::move(symbol); }

    [[nodiscard]] std::size_t size() const noexcept { return m_bars.size(); }
    [[nodiscard]] bool empty() const noexcept { return m_bars.empty(); }
    void reserve(std::size_t n) { m_bars.reserve(n); }

    [[nodiscard]] const Bar& operator[](std::size_t i) const { return m_bars[i]; }
    [[nodiscard]] const std::vector<Bar>& bars() const noexcept { return m_bars; }

    [[nodiscard]] const Bar& front() const { return m_bars.front(); }
    [[nodiscard]] const Bar& back() const { return m_bars.back(); }

    // Appends, requiring strictly increasing timestamps. The legacy loaders
    // accept whatever order the file or query produced, which quietly breaks
    // every rolling-window computation downstream.
    [[nodiscard]] Status append(const Bar& bar) {
        if (!m_bars.empty() && bar.ts <= m_bars.back().ts) {
            return Status(dataCorrupt("BarSeries::append: timestamp " + std::to_string(bar.ts) +
                                      " is not after the previous bar's " +
                                      std::to_string(m_bars.back().ts)));
        }
        if (!bar.isCoherent()) {
            return Status(dataCorrupt("BarSeries::append: incoherent OHLC at ts " +
                                      std::to_string(bar.ts)));
        }
        m_bars.push_back(bar);
        return Status::success();
    }

    // Index of an exact timestamp. O(log n) over the sorted bars, replacing
    // DBMap's parallel hash map.
    [[nodiscard]] Result<std::size_t> indexOf(Timestamp ts) const {
        const auto it = std::lower_bound(
            m_bars.begin(), m_bars.end(), ts,
            [](const Bar& bar, Timestamp value) { return bar.ts < value; });
        if (it == m_bars.end() || it->ts != ts) {
            return notFound("BarSeries::indexOf: no bar at timestamp " + std::to_string(ts));
        }
        return static_cast<std::size_t>(std::distance(m_bars.begin(), it));
    }

    // Index of the last bar at or before ts. What a backtest wants when it
    // asks "where was the market at this moment".
    [[nodiscard]] Result<std::size_t> indexAtOrBefore(Timestamp ts) const {
        if (m_bars.empty() || m_bars.front().ts > ts) {
            return notFound("BarSeries::indexAtOrBefore: nothing at or before " +
                            std::to_string(ts));
        }
        const auto it = std::upper_bound(
            m_bars.begin(), m_bars.end(), ts,
            [](Timestamp value, const Bar& bar) { return value < bar.ts; });
        return static_cast<std::size_t>(std::distance(m_bars.begin(), it) - 1);
    }

    // Verifies the whole series in one pass. Call after loading, before using.
    [[nodiscard]] Status validate() const {
        for (std::size_t i = 0; i < m_bars.size(); ++i) {
            if (!m_bars[i].isCoherent()) {
                return Status(dataCorrupt("BarSeries::validate: incoherent OHLC at index " +
                                          std::to_string(i)));
            }
            if (i > 0 && m_bars[i].ts <= m_bars[i - 1].ts) {
                return Status(dataCorrupt("BarSeries::validate: non-increasing timestamp at "
                                          "index " + std::to_string(i)));
            }
        }
        return Status::success();
    }

    // Gaps larger than the expected spacing, which is what a weekend, a
    // holiday or a feed outage looks like. Patterns must not span these, and
    // Phase 4's embargo logic needs to know where they are.
    [[nodiscard]] std::vector<std::size_t> findGaps(Timestamp expectedSpacingMs) const {
        std::vector<std::size_t> gapIndices;
        for (std::size_t i = 1; i < m_bars.size(); ++i) {
            if (m_bars[i].ts - m_bars[i - 1].ts > expectedSpacingMs) {
                gapIndices.push_back(i);
            }
        }
        return gapIndices;
    }

private:
    std::string m_symbol;
    std::vector<Bar> m_bars;
};

}  // namespace mt::data
