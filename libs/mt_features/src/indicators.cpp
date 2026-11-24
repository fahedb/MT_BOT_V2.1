#include "mt/features/indicators.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace mt::features {
namespace {

constexpr double kTiny = 1e-12;

}  // namespace

data::Series closeSeries(const data::BarSeries& bars) {
    data::Series series("close");
    series.reserve(bars.size());
    for (std::size_t i = 0; i < bars.size(); ++i) {
        series.push(static_cast<float>(bars[i].close));
    }
    return series;
}

data::Series rangeSeries(const data::BarSeries& bars) {
    data::Series series("range");
    series.reserve(bars.size());
    for (std::size_t i = 0; i < bars.size(); ++i) {
        series.push(static_cast<float>(bars[i].high - bars[i].low));
    }
    return series;
}

data::Series bodySeries(const data::BarSeries& bars) {
    data::Series series("body");
    series.reserve(bars.size());
    for (std::size_t i = 0; i < bars.size(); ++i) {
        series.push(static_cast<float>(bars[i].close - bars[i].open));
    }
    return series;
}

data::Series closeLocationSeries(const data::BarSeries& bars) {
    data::Series series("close_location");
    series.reserve(bars.size());
    for (std::size_t i = 0; i < bars.size(); ++i) {
        const double range = bars[i].high - bars[i].low;
        if (range <= kTiny) {
            // A zero-range bar has no "location"; 0.5 would be invented.
            series.pushMissing();
        } else {
            series.push(static_cast<float>((bars[i].close - bars[i].low) / range));
        }
    }
    return series;
}

Result<data::Series> ema(const data::BarSeries& bars, std::size_t period) {
    if (period == 0) {
        return invalidArgument("ema: period must be > 0");
    }

    data::Series series("ema." + std::to_string(period));
    series.reserve(bars.size());

    if (bars.size() < period) {
        // Every value would be missing; say so rather than returning a series
        // of fabricated numbers.
        for (std::size_t i = 0; i < bars.size(); ++i) {
            series.pushMissing();
        }
        return series;
    }

    const double alpha = 2.0 / (static_cast<double>(period) + 1.0);

    // Seed with the simple mean of the first `period` closes.
    double sum = 0.0;
    for (std::size_t i = 0; i < period; ++i) {
        sum += bars[i].close;
        if (i + 1 < period) {
            series.pushMissing();
        }
    }
    double value = sum / static_cast<double>(period);
    series.push(static_cast<float>(value));

    for (std::size_t i = period; i < bars.size(); ++i) {
        value = alpha * bars[i].close + (1.0 - alpha) * value;
        series.push(static_cast<float>(value));
    }
    return series;
}

Result<data::Series> rsi(const data::BarSeries& bars, std::size_t period) {
    if (period == 0) {
        return invalidArgument("rsi: period must be > 0");
    }

    data::Series series("rsi." + std::to_string(period));
    series.reserve(bars.size());

    if (bars.size() <= period) {
        for (std::size_t i = 0; i < bars.size(); ++i) {
            series.pushMissing();
        }
        return series;
    }

    // Wilder's smoothing, seeded from the first `period` changes.
    double gainSum = 0.0;
    double lossSum = 0.0;
    for (std::size_t i = 1; i <= period; ++i) {
        const double change = bars[i].close - bars[i - 1].close;
        gainSum += std::max(change, 0.0);
        lossSum += std::max(-change, 0.0);
    }
    double avgGain = gainSum / static_cast<double>(period);
    double avgLoss = lossSum / static_cast<double>(period);

    for (std::size_t i = 0; i < period; ++i) {
        series.pushMissing();
    }

    const auto rsiFrom = [](double gain, double loss) {
        if (loss <= kTiny) {
            return gain <= kTiny ? 50.0 : 100.0;
        }
        const double rs = gain / loss;
        return 100.0 - (100.0 / (1.0 + rs));
    };

    series.push(static_cast<float>(rsiFrom(avgGain, avgLoss)));

    const double n = static_cast<double>(period);
    for (std::size_t i = period + 1; i < bars.size(); ++i) {
        const double change = bars[i].close - bars[i - 1].close;
        avgGain = (avgGain * (n - 1.0) + std::max(change, 0.0)) / n;
        avgLoss = (avgLoss * (n - 1.0) + std::max(-change, 0.0)) / n;
        series.push(static_cast<float>(rsiFrom(avgGain, avgLoss)));
    }
    return series;
}

Result<data::Series> atr(const data::BarSeries& bars, std::size_t period) {
    if (period == 0) {
        return invalidArgument("atr: period must be > 0");
    }

    data::Series series("atr." + std::to_string(period));
    series.reserve(bars.size());

    if (bars.size() <= period) {
        for (std::size_t i = 0; i < bars.size(); ++i) {
            series.pushMissing();
        }
        return series;
    }

    const auto trueRange = [&bars](std::size_t i) {
        const double previousClose = bars[i - 1].close;
        return std::max({bars[i].high - bars[i].low,
                         std::fabs(bars[i].high - previousClose),
                         std::fabs(bars[i].low - previousClose)});
    };

    double sum = 0.0;
    for (std::size_t i = 1; i <= period; ++i) {
        sum += trueRange(i);
    }
    double value = sum / static_cast<double>(period);

    for (std::size_t i = 0; i < period; ++i) {
        series.pushMissing();
    }
    series.push(static_cast<float>(value));

    const double n = static_cast<double>(period);
    for (std::size_t i = period + 1; i < bars.size(); ++i) {
        value = (value * (n - 1.0) + trueRange(i)) / n;
        series.push(static_cast<float>(value));
    }
    return series;
}

data::Series difference(const data::Series& source) {
    data::Series series(source.name() + ".diff");
    series.reserve(source.size());
    if (source.empty()) {
        return series;
    }

    // No prior sample at index 0, so it is missing. The legacy code wrote 0.0
    // here, which every window containing index 0 then normalised against.
    series.pushMissing();
    for (std::size_t i = 1; i < source.size(); ++i) {
        if (source.isValid(i) && source.isValid(i - 1)) {
            series.push(source[i] - source[i - 1]);
        } else {
            series.pushMissing();
        }
    }
    return series;
}

Result<data::Series> subtract(const data::Series& a, const data::Series& b, std::string name) {
    if (a.size() != b.size()) {
        return invalidArgument("subtract: size mismatch " + std::to_string(a.size()) + " vs " +
                               std::to_string(b.size()));
    }
    data::Series series(std::move(name));
    series.reserve(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a.isValid(i) && b.isValid(i)) {
            series.push(a[i] - b[i]);
        } else {
            series.pushMissing();
        }
    }
    return series;
}

Result<data::Series> divide(const data::Series& a, const data::Series& b, std::string name) {
    if (a.size() != b.size()) {
        return invalidArgument("divide: size mismatch " + std::to_string(a.size()) + " vs " +
                               std::to_string(b.size()));
    }
    data::Series series(std::move(name));
    series.reserve(a.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a.isValid(i) && b.isValid(i) && std::fabs(static_cast<double>(b[i])) > kTiny) {
            series.push(a[i] / b[i]);
        } else {
            series.pushMissing();
        }
    }
    return series;
}

Result<data::Series> rollingPercentileRank(const data::Series& source, std::size_t lookback) {
    if (lookback < 2) {
        return invalidArgument("rollingPercentileRank: lookback must be >= 2");
    }

    data::Series series(source.name() + ".pct");
    series.reserve(source.size());

    for (std::size_t i = 0; i < source.size(); ++i) {
        if (i + 1 < lookback || !source.isValid(i)) {
            series.pushMissing();
            continue;
        }
        const std::size_t begin = i + 1 - lookback;
        std::size_t below = 0;
        std::size_t counted = 0;
        for (std::size_t j = begin; j <= i; ++j) {
            if (!source.isValid(j)) {
                continue;
            }
            ++counted;
            if (source[j] < source[i]) {
                ++below;
            }
        }
        if (counted < 2) {
            series.pushMissing();
        } else {
            series.push(static_cast<float>(static_cast<double>(below) /
                                           static_cast<double>(counted - 1)));
        }
    }
    return series;
}

}  // namespace mt::features
