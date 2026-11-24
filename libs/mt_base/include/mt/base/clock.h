// ---------------------------------------------------------------------------
// Time as an interface.
//
// The live scanner reads the wall clock directly, which is why a backtest
// cannot drive it deterministically and why the loop sleeps instead of waiting
// on a condition (Phase 6 of docs/REFACTOR_PLAN.md). Components take an
// IClock& so a backtest can supply simulated time and a test can step it.
// ---------------------------------------------------------------------------
#pragma once

#include <chrono>
#include <cstdint>

namespace mt {

// Market timestamps are kept as the project already keeps them: milliseconds
// since the Unix epoch in a 64-bit integer, matching ChangeNode's
// point_DateTimeLong so conversion at the boundary stays trivial.
using Timestamp = std::int64_t;

class IClock {
public:
    virtual ~IClock() = default;

    // Wall-clock time, for logging and staleness checks.
    [[nodiscard]] virtual Timestamp nowMs() const = 0;

    // Monotonic time, for measuring durations. Never goes backwards.
    [[nodiscard]] virtual std::chrono::steady_clock::time_point steadyNow() const = 0;
};

class SystemClock final : public IClock {
public:
    [[nodiscard]] Timestamp nowMs() const override {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    [[nodiscard]] std::chrono::steady_clock::time_point steadyNow() const override {
        return std::chrono::steady_clock::now();
    }

    static SystemClock& instance() {
        static SystemClock clock;
        return clock;
    }
};

// Time under test control: starts where you put it and only moves when you
// advance it.
class ManualClock final : public IClock {
public:
    explicit ManualClock(Timestamp startMs = 0) : m_nowMs(startMs) {}

    [[nodiscard]] Timestamp nowMs() const override { return m_nowMs; }

    [[nodiscard]] std::chrono::steady_clock::time_point steadyNow() const override {
        return m_steadyBase + std::chrono::milliseconds(m_nowMs);
    }

    void advanceMs(Timestamp delta) { m_nowMs += delta; }
    void setMs(Timestamp value) { m_nowMs = value; }

private:
    Timestamp m_nowMs;
    std::chrono::steady_clock::time_point m_steadyBase{};
};

// Convenience for timing a block without reaching for a global.
class ScopedDuration {
public:
    explicit ScopedDuration(const IClock& clock)
        : m_clock(clock), m_start(clock.steadyNow()) {}

    [[nodiscard]] std::int64_t elapsedMs() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   m_clock.steadyNow() - m_start)
            .count();
    }

    [[nodiscard]] std::int64_t elapsedUs() const {
        return std::chrono::duration_cast<std::chrono::microseconds>(
                   m_clock.steadyNow() - m_start)
            .count();
    }

private:
    const IClock& m_clock;
    std::chrono::steady_clock::time_point m_start;
};

}  // namespace mt
