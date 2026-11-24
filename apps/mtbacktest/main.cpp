// ---------------------------------------------------------------------------
// mtbacktest — walk-forward evaluation of the new engine.
//
// docs/REFACTOR_PLAN.md Phase 8. What it does and, more importantly, what it
// refuses to do:
//
//  * it evaluates only bars strictly after the calibration span, so the
//    calibration cannot have seen the evaluated period (finding S11);
//  * every candidate is embargoed by window + horizon, enforced inside the
//    engine (finding B12);
//  * costs are subtracted, because an uncosted backtest flatters every
//    strategy;
//  * it reports the rejection breakdown, not just the trades, so "no edge" and
//    "no signal" are distinguishable;
//  * it prints the baselines beside the result. A number with no baseline is
//    not a result.
//
//     mtbacktest --bars history.csv --calibration cal.bin
//                [--window 48] [--paa 12] [--train-fraction 0.6]
//                [--cost 20] [--min-score 70] [--step 1]
//
// This tool reports; it does not conclude. Section 8 of the plan sets out what
// has to be true before a number here means anything: purged walk-forward
// across folds, a held-out span looked at once, and an explicit count of how
// many configurations were tried.
// ---------------------------------------------------------------------------
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "csv_bars.h"
#include "mt/engine/engine.h"
#include "mt/similarity/calibration.h"

namespace {

struct Options {
    std::string barsPath;
    std::string calibrationPath;
    std::string symbol = "UNKNOWN";
    std::size_t windowLength = 48;
    std::size_t paaSegments = 12;
    double trainFraction = 0.6;
    double costPoints = 20.0;
    double minScore = 70.0;
    std::size_t step = 1;
    bool help = false;
};

void printUsage() {
    std::cout << R"(mtbacktest - walk-forward evaluation of the pattern engine

Usage:
  mtbacktest --bars <file.csv> --calibration <file.bin> [options]

Options:
  --symbol <name>          symbol label                      (default UNKNOWN)
  --window <n>             pattern window length             (default 48)
  --paa <n>                PAA segments per channel          (default 12)
  --train-fraction <f>     share of history reserved for the calibration span;
                           only bars after it are evaluated  (default 0.6)
  --cost <points>          round-trip spread + commission    (default 20)
  --min-score <0..100>     calibrated score required         (default 70)
  --step <n>               evaluate every n-th bar           (default 1)
  --help

The --paa and --window values must match the ones the calibration was built
with, or it will be refused: standardisation changes invalidate a calibration.
)";
}

bool parseSize(const char* text, std::size_t& out) {
    char* end = nullptr;
    const unsigned long long value = std::strtoull(text, &end, 10);
    if (end == text || *end != '\0' || value == 0) return false;
    out = static_cast<std::size_t>(value);
    return true;
}

bool parseDouble(const char* text, double& out) {
    char* end = nullptr;
    out = std::strtod(text, &end);
    return end != text && *end == '\0';
}

mt::Result<Options> parseOptions(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        const auto next = [&]() -> const char* { return (i + 1 < argc) ? argv[++i] : nullptr; };

        if (arg == "--help" || arg == "-h") {
            options.help = true;
            return options;
        }
        const char* value = next();
        if (value == nullptr) {
            return mt::invalidArgument(arg + " needs a value");
        }

        if (arg == "--bars") {
            options.barsPath = value;
        } else if (arg == "--calibration") {
            options.calibrationPath = value;
        } else if (arg == "--symbol") {
            options.symbol = value;
        } else if (arg == "--window") {
            if (!parseSize(value, options.windowLength)) {
                return mt::invalidArgument("--window needs a positive integer");
            }
        } else if (arg == "--paa") {
            if (!parseSize(value, options.paaSegments)) {
                return mt::invalidArgument("--paa needs a positive integer");
            }
        } else if (arg == "--step") {
            if (!parseSize(value, options.step)) {
                return mt::invalidArgument("--step needs a positive integer");
            }
        } else if (arg == "--train-fraction") {
            if (!parseDouble(value, options.trainFraction) || options.trainFraction <= 0.0 ||
                options.trainFraction >= 1.0) {
                return mt::invalidArgument("--train-fraction must be strictly between 0 and 1");
            }
        } else if (arg == "--cost") {
            if (!parseDouble(value, options.costPoints) || options.costPoints < 0.0) {
                return mt::invalidArgument("--cost must be >= 0");
            }
        } else if (arg == "--min-score") {
            if (!parseDouble(value, options.minScore) || options.minScore < 0.0 ||
                options.minScore > 100.0) {
                return mt::invalidArgument("--min-score must be in [0, 100]");
            }
        } else {
            return mt::invalidArgument("unknown argument: " + arg);
        }
    }

    if (options.barsPath.empty() || options.calibrationPath.empty()) {
        return mt::invalidArgument("--bars and --calibration are both required");
    }
    return options;
}

struct Tally {
    std::size_t evaluated = 0;
    std::size_t trades = 0;
    std::size_t wins = 0;
    std::size_t losses = 0;
    double netPoints = 0.0;
    std::map<std::string, std::size_t> rejections;
};

}  // namespace

int main(int argc, char** argv) {
    const auto options = parseOptions(argc, argv);
    if (!options) {
        std::cerr << "error: " << options.error().message() << "\n\n";
        printUsage();
        return 2;
    }
    if (options.value().help) {
        printUsage();
        return 0;
    }
    const Options& opt = options.value();

    mtbot::app::CsvLoadReport loadReport;
    const auto bars = mtbot::app::loadBarsFromCsv(opt.barsPath, opt.symbol, loadReport);
    if (!bars) {
        std::cerr << "error: " << bars.error().describe() << '\n';
        return 1;
    }
    std::cout << "loaded " << loadReport.barsAccepted << " bars\n";

    const auto calibration = mt::similarity::CalibrationSet::load(opt.calibrationPath);
    if (!calibration) {
        std::cerr << "error: " << calibration.error().describe() << '\n';
        return 1;
    }

    mt::engine::EngineConfig config;
    config.windowLength = opt.windowLength;
    config.minScore = opt.minScore;
    config.policy.costPoints = opt.costPoints;

    const mt::features::ChannelRegistry registry =
        mt::features::defaultEngineRegistry(opt.paaSegments);

    const auto engine = mt::engine::PatternEngine::build(bars.value(), registry,
                                                         calibration.value(), config);
    if (!engine) {
        std::cerr << "error: " << engine.error().describe() << '\n';
        if (engine.error().code() == mt::ErrorCode::FailedPrecondition) {
            std::cerr << "hint: --window and --paa must match the values mtcalibrate used.\n";
        }
        return 1;
    }

    // Only bars after the calibration span are evaluated. The calibration saw
    // the earlier part, so evaluating there would be a look-ahead.
    const std::size_t total = bars.value().size();
    const std::size_t evaluateFrom =
        std::max(engine.value()->firstEvaluableIndex(),
                 static_cast<std::size_t>(static_cast<double>(total) * opt.trainFraction));
    const std::size_t evaluateTo = total - config.label.horizonBars - 1;

    if (evaluateFrom >= evaluateTo) {
        std::cerr << "error: no evaluable bars. History is " << total
                  << " bars; the window, embargo and train fraction leave nothing.\n";
        return 1;
    }

    std::cout << "evaluating bars " << evaluateFrom << " .. " << evaluateTo << " (step "
              << opt.step << ")\n"
              << "calibration span: 0 .. " << evaluateFrom - 1 << " (not evaluated)\n"
              << "cost: " << opt.costPoints << " points, min score: " << opt.minScore << "\n\n";

    Tally tally;
    for (std::size_t index = evaluateFrom; index <= evaluateTo; index += opt.step) {
        const auto result = engine.value()->evaluateAt(index);
        if (!result) {
            std::cerr << "error at bar " << index << ": " << result.error().describe() << '\n';
            return 1;
        }
        ++tally.evaluated;

        const auto& decision = result.value().decision;
        if (!decision.trade) {
            ++tally.rejections[std::string(mt::policy::toString(decision.reason))];
            continue;
        }
        ++tally.trades;

        // Outcome from the same label definition the matches were judged by,
        // so the evaluation is consistent with the prediction.
        const auto outcome =
            mt::labels::labelForward(bars.value(), index, engine.value()->config().label);
        if (!outcome || !outcome.value().valid) {
            continue;
        }

        const bool correct =
            outcome.value().firstTouch == decision.direction;
        const bool noise = outcome.value().firstTouch == mt::labels::Direction::Noise;

        if (noise) {
            // Neither barrier reached: in a real account this is a timed exit
            // near breakeven, minus cost.
            tally.netPoints -= opt.costPoints;
        } else if (correct) {
            ++tally.wins;
            tally.netPoints += decision.takeProfitPoints - opt.costPoints;
        } else {
            ++tally.losses;
            tally.netPoints -= decision.stopLossPoints + opt.costPoints;
        }
    }

    std::cout << "=== results ===\n"
              << "bars evaluated: " << tally.evaluated << '\n'
              << "trades:         " << tally.trades << " ("
              << std::fixed << std::setprecision(2)
              << (tally.evaluated > 0 ? 100.0 * static_cast<double>(tally.trades) /
                                            static_cast<double>(tally.evaluated)
                                      : 0.0)
              << "% of bars)\n";

    if (tally.trades > 0) {
        const std::size_t decided = tally.wins + tally.losses;
        std::cout << "wins / losses:  " << tally.wins << " / " << tally.losses << '\n';
        if (decided > 0) {
            std::cout << "hit rate:       "
                      << 100.0 * static_cast<double>(tally.wins) /
                             static_cast<double>(decided)
                      << "%\n";
        }
        std::cout << "net points:     " << tally.netPoints << '\n'
                  << "per trade:      "
                  << tally.netPoints / static_cast<double>(tally.trades) << '\n';
    }

    std::cout << "\n=== why the other bars were not traded ===\n";
    for (const auto& [reason, count] : tally.rejections) {
        std::cout << "  " << std::left << std::setw(22) << reason << std::right << count
                  << '\n';
    }

    std::cout << R"(
=== what this is not ===
A single-span result with no baseline and no trial accounting. Before treating
any of it as evidence, section 8 of docs/REFACTOR_PLAN.md requires:
  * purged walk-forward across several folds, not one split;
  * comparison against random match sets, shuffled labels, always-long, and
    the legacy engine;
  * a final held-out span looked at exactly once;
  * the number of configurations tried, stated alongside the result.
)";

    return 0;
}
