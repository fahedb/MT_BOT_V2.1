#include "mt/labels/label.h"

#include <algorithm>
#include <cmath>

namespace mt::labels {

std::string_view toString(Direction direction) noexcept {
    switch (direction) {
        case Direction::Up:    return "UP";
        case Direction::Down:  return "DOWN";
        case Direction::Noise: return "NOISE";
    }
    return "NOISE";
}

Direction ForwardLabel::primaryDirection(LabelKind kind) const noexcept {
    switch (kind) {
        case LabelKind::FirstTouch: return firstTouch;
        case LabelKind::NetReturn:  return netDirection;
        case LabelKind::MaxFavourableExcursion:
            // Direction of the larger excursion. Reported for completeness;
            // it is a weak label because it ignores which came first.
            return (maxFavourablePoints >= -maxAdversePoints) ? Direction::Up : Direction::Down;
    }
    return Direction::Noise;
}

Result<ForwardLabel> labelForward(const data::BarSeries& bars, std::size_t entryIndex,
                                  const LabelConfig& config) {
    if (config.horizonBars == 0) {
        return invalidArgument("labelForward: horizonBars must be > 0");
    }
    if (!(config.touchPoints > 0.0) || !(config.pointSize > 0.0)) {
        return invalidArgument("labelForward: touchPoints and pointSize must be > 0");
    }
    if (entryIndex >= bars.size()) {
        return outOfRange("labelForward: entryIndex " + std::to_string(entryIndex) +
                          " beyond " + std::to_string(bars.size()));
    }

    ForwardLabel label;

    // The horizon begins at the bar *after* the pattern ends, so no part of
    // the label can overlap the pattern it describes.
    const std::size_t firstForward = entryIndex + 1;
    const std::size_t lastForward = entryIndex + config.horizonBars;
    if (lastForward >= bars.size()) {
        // Not enough future: a normal condition at the right edge of history,
        // and the reason the most recent bars can never be training data.
        label.valid = false;
        return label;
    }

    const double entryClose = bars[entryIndex].close;
    const double barrier = config.touchPoints * config.pointSize;
    const double upLevel = entryClose + barrier;
    const double downLevel = entryClose - barrier;

    double highest = entryClose;
    double lowest = entryClose;
    bool touched = false;

    for (std::size_t i = firstForward; i <= lastForward; ++i) {
        // A horizon that spans a weekend or a feed outage is not a horizon.
        if (bars[i].ts - bars[i - 1].ts > config.maxBarGapMs) {
            label.valid = false;
            return label;
        }

        const data::Bar& bar = bars[i];
        highest = std::max(highest, bar.high);
        lowest = std::min(lowest, bar.low);

        const bool hitUp = bar.high >= upLevel;
        const bool hitDown = bar.low <= downLevel;

        if (hitUp && hitDown) {
            // Both barriers inside one bar. OHLC cannot order them.
            label.sameCandleAmbiguity = true;
            label.barsToTouch = i - entryIndex;
            switch (config.sameCandlePolicy) {
                case SameCandlePolicy::Noise:     label.firstTouch = Direction::Noise; break;
                case SameCandlePolicy::UpFirst:   label.firstTouch = Direction::Up; break;
                case SameCandlePolicy::DownFirst: label.firstTouch = Direction::Down; break;
            }
            touched = true;
            break;
        }
        if (hitUp) {
            label.firstTouch = Direction::Up;
            label.barsToTouch = i - entryIndex;
            touched = true;
            break;
        }
        if (hitDown) {
            label.firstTouch = Direction::Down;
            label.barsToTouch = i - entryIndex;
            touched = true;
            break;
        }
    }

    if (!touched) {
        // Neither barrier reached inside the horizon. That is information, not
        // a missing value: the move was too small to trade.
        label.firstTouch = Direction::Noise;
        label.barsToTouch = config.horizonBars;
    }

    const double netMove = bars[lastForward].close - entryClose;
    label.netReturnPoints = netMove / config.pointSize;
    label.netDirection = (std::fabs(netMove) < barrier)
                             ? Direction::Noise
                             : (netMove > 0.0 ? Direction::Up : Direction::Down);

    label.maxFavourablePoints = (highest - entryClose) / config.pointSize;
    label.maxAdversePoints = (lowest - entryClose) / config.pointSize;
    label.valid = true;
    return label;
}

}  // namespace mt::labels
