#include "mt/features/standardise.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace mt::features {
namespace {

// Collects the valid samples. Small windows (tens of samples), so the copy is
// cheaper than any cleverness, and it keeps the statistics obviously correct.
std::vector<float> validSamples(const data::WindowView& window) {
    std::vector<float> out;
    out.reserve(window.size());
    for (std::size_t i = 0; i < window.size(); ++i) {
        if (window.isValid(i)) {
            out.push_back(window[i]);
        }
    }
    return out;
}

float medianOfSorted(std::vector<float>& samples) {
    const std::size_t n = samples.size();
    const std::size_t mid = n / 2;
    std::nth_element(samples.begin(), samples.begin() + static_cast<std::ptrdiff_t>(mid),
                     samples.end());
    const float upper = samples[mid];
    if (n % 2 == 1) {
        return upper;
    }
    // Even count: average the two central order statistics. max_element over
    // the lower half is exact and avoids a second nth_element.
    const float lower =
        *std::max_element(samples.begin(), samples.begin() + static_cast<std::ptrdiff_t>(mid));
    return 0.5F * (lower + upper);
}

constexpr float kMadToSigma = 1.4826F;

}  // namespace

std::optional<float> medianOfValid(const data::WindowView& window) {
    std::vector<float> samples = validSamples(window);
    if (samples.empty()) {
        return std::nullopt;
    }
    return medianOfSorted(samples);
}

std::optional<float> robustSigmaOfValid(const data::WindowView& window, float median) {
    std::vector<float> deviations;
    deviations.reserve(window.size());
    for (std::size_t i = 0; i < window.size(); ++i) {
        if (window.isValid(i)) {
            deviations.push_back(std::fabs(window[i] - median));
        }
    }
    if (deviations.empty()) {
        return std::nullopt;
    }
    return kMadToSigma * medianOfSorted(deviations);
}

std::optional<float> meanOfValid(const data::WindowView& window) {
    double sum = 0.0;
    std::size_t n = 0;
    for (std::size_t i = 0; i < window.size(); ++i) {
        if (window.isValid(i)) {
            sum += static_cast<double>(window[i]);
            ++n;
        }
    }
    if (n == 0) {
        return std::nullopt;
    }
    return static_cast<float>(sum / static_cast<double>(n));
}

std::optional<float> stdDevOfValid(const data::WindowView& window, float mean) {
    double sumSq = 0.0;
    std::size_t n = 0;
    for (std::size_t i = 0; i < window.size(); ++i) {
        if (window.isValid(i)) {
            const double d = static_cast<double>(window[i]) - static_cast<double>(mean);
            sumSq += d * d;
            ++n;
        }
    }
    if (n < 2) {
        return std::nullopt;
    }
    return static_cast<float>(std::sqrt(sumSq / static_cast<double>(n - 1)));
}

std::optional<MinMaxOfValid> minMaxOfValid(const data::WindowView& window) {
    std::optional<MinMaxOfValid> result;
    for (std::size_t i = 0; i < window.size(); ++i) {
        if (!window.isValid(i)) {
            continue;
        }
        const float v = window[i];
        if (!result.has_value()) {
            result = MinMaxOfValid{v, v};  // seeded from data, never from a constant
        } else {
            result->min = std::min(result->min, v);
            result->max = std::max(result->max, v);
        }
    }
    return result;
}

Result<std::vector<float>> piecewiseAggregate(const std::vector<float>& values,
                                              std::size_t segments) {
    if (segments == 0) {
        return invalidArgument("piecewiseAggregate: segments must be > 0");
    }
    if (values.empty()) {
        return invalidArgument("piecewiseAggregate: empty input");
    }
    if (segments > values.size()) {
        return invalidArgument("piecewiseAggregate: " + std::to_string(segments) +
                               " segments requested for only " + std::to_string(values.size()) +
                               " samples");
    }

    std::vector<float> out;
    out.reserve(segments);

    // Boundaries by exact proportion, so every sample lands in exactly one
    // segment and no segment is empty.
    for (std::size_t s = 0; s < segments; ++s) {
        const std::size_t begin = (s * values.size()) / segments;
        const std::size_t end = ((s + 1) * values.size()) / segments;
        double sum = 0.0;
        for (std::size_t i = begin; i < end; ++i) {
            sum += static_cast<double>(values[i]);
        }
        out.push_back(static_cast<float>(sum / static_cast<double>(end - begin)));
    }
    return out;
}

namespace {

// Applies the configured mode. Returns nullopt when no usable scale exists.
std::optional<std::vector<float>> applyMode(const data::WindowView& window,
                                            const StandardiseConfig& config) {
    std::vector<float> out(window.size(), 0.0F);

    switch (config.mode) {
        case StandardiseMode::None: {
            for (std::size_t i = 0; i < window.size(); ++i) {
                out[i] = window.isValid(i) ? window[i] : 0.0F;
            }
            return out;
        }

        case StandardiseMode::CentreOnly: {
            const std::optional<float> median = medianOfValid(window);
            if (!median.has_value()) {
                return std::nullopt;
            }
            const double med = static_cast<double>(*median);
            for (std::size_t i = 0; i < window.size(); ++i) {
                out[i] = window.isValid(i)
                             ? static_cast<float>(static_cast<double>(window[i]) - med)
                             : 0.0F;
            }
            return out;
        }

        case StandardiseMode::MinMax: {
            const std::optional<MinMaxOfValid> mm = minMaxOfValid(window);
            if (!mm.has_value()) {
                return std::nullopt;
            }
            const double lo = static_cast<double>(mm->min);
            const double range = static_cast<double>(mm->max) - lo;
            if (range < static_cast<double>(config.epsilon)) {
                // A flat window has no shape to speak of; report it as flat
                // rather than dividing by epsilon and amplifying noise.
                return out;
            }
            for (std::size_t i = 0; i < window.size(); ++i) {
                out[i] = window.isValid(i)
                             ? static_cast<float>((static_cast<double>(window[i]) - lo) / range)
                             : 0.0F;
            }
            return out;
        }

        case StandardiseMode::MeanZ: {
            const std::optional<float> mean = meanOfValid(window);
            if (!mean.has_value()) {
                return std::nullopt;
            }
            const std::optional<float> sd = stdDevOfValid(window, *mean);
            if (!sd.has_value() || *sd < config.epsilon) {
                return out;  // flat
            }
            const double avg = static_cast<double>(*mean);
            const double scale = static_cast<double>(*sd);
            const double clamp = static_cast<double>(config.clampSigma);
            for (std::size_t i = 0; i < window.size(); ++i) {
                const double z =
                    window.isValid(i) ? ((static_cast<double>(window[i]) - avg) / scale) : 0.0;
                out[i] = static_cast<float>(std::clamp(z, -clamp, clamp));
            }
            return out;
        }

        case StandardiseMode::RobustZ: {
            const std::optional<float> median = medianOfValid(window);
            if (!median.has_value()) {
                return std::nullopt;
            }
            const std::optional<float> sigma = robustSigmaOfValid(window, *median);
            if (!sigma.has_value() || *sigma < config.epsilon) {
                // MAD is zero when over half the samples share one value: a
                // flat or heavily quantised window. Flat is the honest answer.
                return out;
            }
            // Centring and scaling in double. The inputs are float32, so the
            // residual (x - median) is small relative to x and would lose
            // significance if computed in float: at a price of 18000, float32
            // steps are ~0.002, which is a meaningful fraction of a typical
            // window sigma. Doing it in double leaves only the inputs' own
            // representation error.
            const double med = static_cast<double>(*median);
            const double scale = static_cast<double>(*sigma);
            const double clamp = static_cast<double>(config.clampSigma);
            for (std::size_t i = 0; i < window.size(); ++i) {
                const double z =
                    window.isValid(i) ? ((static_cast<double>(window[i]) - med) / scale) : 0.0;
                out[i] = static_cast<float>(std::clamp(z, -clamp, clamp));
            }
            return out;
        }
    }
    return std::nullopt;
}

}  // namespace

Result<StandardisedWindow> standardise(const data::WindowView& window,
                                       const StandardiseConfig& config) {
    if (window.empty()) {
        return invalidArgument("standardise: empty window");
    }
    if (config.clampSigma <= 0.0F) {
        return invalidArgument("standardise: clampSigma must be > 0");
    }
    if (config.epsilon <= 0.0F) {
        return invalidArgument("standardise: epsilon must be > 0");
    }

    StandardisedWindow result;

    if (window.validRatio() < config.minValidRatio) {
        // Not an error: an incomplete window is a normal event at a data gap.
        // It is simply not usable, and the caller redistributes its weight.
        result.usable = false;
        return result;
    }

    std::optional<std::vector<float>> scaled = applyMode(window, config);
    if (!scaled.has_value()) {
        result.usable = false;
        return result;
    }

    if (config.paaSegments > 0) {
        Result<std::vector<float>> reduced = piecewiseAggregate(*scaled, config.paaSegments);
        if (!reduced) {
            return reduced.error();
        }
        result.values = std::move(reduced).value();
    } else {
        result.values = std::move(*scaled);
    }

    result.usable = true;
    return result;
}

Result<StandardisedWindow> standardiseScaled(const data::WindowView& window, float scale,
                                             const StandardiseConfig& config) {
    if (!(scale > 0.0F)) {
        return invalidArgument("standardiseScaled: scale must be > 0, got " +
                               std::to_string(scale));
    }

    // Pre-divide into a local copy, keeping the validity bits.
    std::vector<float> values(window.size());
    std::vector<std::uint8_t> valid(window.size());
    for (std::size_t i = 0; i < window.size(); ++i) {
        valid[i] = window.isValid(i) ? std::uint8_t{1} : std::uint8_t{0};
        values[i] = window.isValid(i) ? (window[i] / scale) : 0.0F;
    }

    const data::WindowView scaledView(values, valid);
    return standardise(scaledView, config);
}

}  // namespace mt::features
