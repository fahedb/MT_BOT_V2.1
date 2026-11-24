// ---------------------------------------------------------------------------
// Series — one channel of numbers over time, with an explicit validity mask.
//
// This is the fix for finding B6. The pre-refactor code wrote -1.0f into the
// value arrays whenever an indicator was missing
// (TimeLineArrayBuilder.cpp:52,57), and those arrays then fed the rolling
// min/max and the similarity comparison. One gap silently corrupted a window's
// normalisation and injected a fake bar into every comparison against it.
//
// Here, missing is a separate bit. Nothing downstream can mistake it for data,
// and a window that is too incomplete to use says so instead of scoring.
//
// Value semantics throughout: no vector<T*> pointing into a separately-owned
// pool that can reallocate underneath it (finding E8).
// ---------------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "mt/base/error.h"

namespace mt::data {

// A read-only view over a contiguous stretch of a Series, carrying its
// validity bits. Cheap to pass; it does not own anything.
class WindowView {
public:
    WindowView() = default;

    WindowView(std::span<const float> values, std::span<const std::uint8_t> valid)
        : m_values(values), m_valid(valid) {}

    [[nodiscard]] std::size_t size() const noexcept { return m_values.size(); }
    [[nodiscard]] bool empty() const noexcept { return m_values.empty(); }

    [[nodiscard]] float operator[](std::size_t i) const { return m_values[i]; }
    [[nodiscard]] bool isValid(std::size_t i) const { return m_valid[i] != 0; }

    [[nodiscard]] std::span<const float> values() const noexcept { return m_values; }
    [[nodiscard]] std::span<const std::uint8_t> validity() const noexcept { return m_valid; }

    [[nodiscard]] std::size_t validCount() const noexcept {
        std::size_t n = 0;
        for (const std::uint8_t v : m_valid) {
            n += (v != 0) ? 1u : 0u;
        }
        return n;
    }

    [[nodiscard]] double validRatio() const noexcept {
        return m_values.empty() ? 0.0
                                : static_cast<double>(validCount()) /
                                      static_cast<double>(m_values.size());
    }

    [[nodiscard]] bool allValid() const noexcept { return validCount() == m_values.size(); }

private:
    std::span<const float> m_values;
    std::span<const std::uint8_t> m_valid;
};

class Series {
public:
    Series() = default;
    explicit Series(std::string name) : m_name(std::move(name)) {}

    Series(std::string name, std::vector<float> values)
        : m_name(std::move(name)),
          m_values(std::move(values)),
          m_valid(m_values.size(), std::uint8_t{1}) {}

    [[nodiscard]] const std::string& name() const noexcept { return m_name; }
    void setName(std::string name) { m_name = std::move(name); }

    [[nodiscard]] std::size_t size() const noexcept { return m_values.size(); }
    [[nodiscard]] bool empty() const noexcept { return m_values.empty(); }

    void reserve(std::size_t n) {
        m_values.reserve(n);
        m_valid.reserve(n);
    }

    void push(float value) {
        m_values.push_back(value);
        m_valid.push_back(1);
    }

    // The honest alternative to a sentinel: the slot exists, the value does
    // not. The stored number is never read, but it is zero rather than
    // uninitialised so that a mistaken read is at least deterministic.
    void pushMissing() {
        m_values.push_back(0.0f);
        m_valid.push_back(0);
    }

    void set(std::size_t i, float value) {
        m_values[i] = value;
        m_valid[i] = 1;
    }

    void setMissing(std::size_t i) {
        m_values[i] = 0.0f;
        m_valid[i] = 0;
    }

    [[nodiscard]] float operator[](std::size_t i) const { return m_values[i]; }
    [[nodiscard]] bool isValid(std::size_t i) const { return m_valid[i] != 0; }

    [[nodiscard]] std::size_t validCount() const noexcept {
        std::size_t n = 0;
        for (const std::uint8_t v : m_valid) {
            n += (v != 0) ? 1u : 0u;
        }
        return n;
    }

    [[nodiscard]] std::span<const float> values() const noexcept { return m_values; }
    [[nodiscard]] std::span<const std::uint8_t> validity() const noexcept { return m_valid; }

    // The `length` samples ending at and including `endIndex`, i.e. the window
    // a pattern at that point is made of. Out-of-range is an error, not a
    // clamp: silently shortening a pattern window is how comparisons end up
    // being made between different numbers of bars.
    [[nodiscard]] Result<WindowView> window(std::size_t endIndex, std::size_t length) const {
        if (length == 0) {
            return invalidArgument("Series::window: length must be > 0");
        }
        if (endIndex >= m_values.size()) {
            return outOfRange("Series::window: endIndex " + std::to_string(endIndex) +
                              " beyond size " + std::to_string(m_values.size()));
        }
        if (endIndex + 1 < length) {
            return outOfRange("Series::window: not enough history before index " +
                              std::to_string(endIndex) + " for length " +
                              std::to_string(length));
        }
        const std::size_t begin = endIndex + 1 - length;
        return WindowView(std::span<const float>(m_values).subspan(begin, length),
                          std::span<const std::uint8_t>(m_valid).subspan(begin, length));
    }

    // Window identified by its first index instead of its last, which is how
    // the historical scan enumerates candidates.
    [[nodiscard]] Result<WindowView> windowFrom(std::size_t beginIndex,
                                                std::size_t length) const {
        if (length == 0) {
            return invalidArgument("Series::windowFrom: length must be > 0");
        }
        if (beginIndex + length > m_values.size()) {
            return outOfRange("Series::windowFrom: window [" + std::to_string(beginIndex) +
                              ", " + std::to_string(beginIndex + length) +
                              ") exceeds size " + std::to_string(m_values.size()));
        }
        return WindowView(std::span<const float>(m_values).subspan(beginIndex, length),
                          std::span<const std::uint8_t>(m_valid).subspan(beginIndex, length));
    }

    // Number of distinct windows of this length that fit, i.e. the candidate
    // count for a scan. The legacy code spelled this `histSize - patternLen + 1`
    // in a dozen places and got the edge cases wrong in some of them.
    [[nodiscard]] std::size_t windowCount(std::size_t length) const noexcept {
        return (length == 0 || m_values.size() < length) ? 0
                                                         : m_values.size() - length + 1;
    }

private:
    std::string m_name;
    std::vector<float> m_values;
    std::vector<std::uint8_t> m_valid;
};

}  // namespace mt::data
