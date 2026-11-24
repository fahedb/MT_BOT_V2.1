// ---------------------------------------------------------------------------
// mtcalibrate — builds the calibration artefact the scorer needs.
//
// docs/REFACTOR_PLAN.md section 6.4 and Phase 3. Without a calibration there
// are no comparable similarities and therefore no meaningful score, so this is
// the first tool to run against real history.
//
//     mtcalibrate --bars history.csv --out config/calibration/xauusd_v1.bin
//                 [--window 48] [--paa 12] [--pairs 200000] [--bins 1024]
//                 [--symbol XAUUSD] [--seed 1234]
//
// Fit it on TRAINING data only. A calibration built over a span that includes
// the period you then evaluate on is a look-ahead: the null distribution would
// already know about the future (plan finding S11).
// ---------------------------------------------------------------------------
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#include "csv_bars.h"
#include "mt/features/channel_set.h"
#include "mt/similarity/calibrate.h"

namespace {

struct Options {
    std::string barsPath;
    std::string outPath;
    std::string symbol = "UNKNOWN";
    std::size_t windowLength = 48;
    std::size_t paaSegments = 12;
    std::size_t pairCount = 200000;
    std::size_t bins = mt::similarity::ChannelCalibration::kDefaultBins;
    std::uint64_t seed = 0x5EEDCA11;
    bool help = false;
};

void printUsage() {
    std::cout << R"(mtcalibrate - build a calibration artefact from historical bars

Usage:
  mtcalibrate --bars <file.csv> --out <file.bin> [options]

Required:
  --bars <path>     CSV: timestamp,open,high,low,close[,volume]
                    timestamp is epoch milliseconds or "YYYY-MM-DD HH:MM:SS"
  --out <path>      where to write the calibration

Options:
  --symbol <name>   symbol label recorded in the series   (default UNKNOWN)
  --window <n>      pattern window length in bars         (default 48)
  --paa <n>         PAA segments per channel              (default 12)
  --pairs <n>       random window pairs to sample         (default 200000)
  --bins <n>        quantile bins per channel             (default 1024)
  --seed <n>        sampling seed, for reproducibility    (default 1414742545)
  --help

Fit this on TRAINING data only: a calibration spanning the period you evaluate
on is a look-ahead.
)";
}

bool parseSize(const char* text, std::size_t& out) {
    char* end = nullptr;
    const unsigned long long value = std::strtoull(text, &end, 10);
    if (end == text || *end != '\0' || value == 0) {
        return false;
    }
    out = static_cast<std::size_t>(value);
    return true;
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
        if (arg == "--bars") {
            const char* value = next();
            if (value == nullptr) return mt::invalidArgument("--bars needs a path");
            options.barsPath = value;
        } else if (arg == "--out") {
            const char* value = next();
            if (value == nullptr) return mt::invalidArgument("--out needs a path");
            options.outPath = value;
        } else if (arg == "--symbol") {
            const char* value = next();
            if (value == nullptr) return mt::invalidArgument("--symbol needs a name");
            options.symbol = value;
        } else if (arg == "--window") {
            const char* value = next();
            if (value == nullptr || !parseSize(value, options.windowLength)) {
                return mt::invalidArgument("--window needs a positive integer");
            }
        } else if (arg == "--paa") {
            const char* value = next();
            if (value == nullptr || !parseSize(value, options.paaSegments)) {
                return mt::invalidArgument("--paa needs a positive integer");
            }
        } else if (arg == "--pairs") {
            const char* value = next();
            if (value == nullptr || !parseSize(value, options.pairCount)) {
                return mt::invalidArgument("--pairs needs a positive integer");
            }
        } else if (arg == "--bins") {
            const char* value = next();
            if (value == nullptr || !parseSize(value, options.bins)) {
                return mt::invalidArgument("--bins needs a positive integer");
            }
        } else if (arg == "--seed") {
            const char* value = next();
            std::size_t seed = 0;
            if (value == nullptr || !parseSize(value, seed)) {
                return mt::invalidArgument("--seed needs a positive integer");
            }
            options.seed = static_cast<std::uint64_t>(seed);
        } else {
            return mt::invalidArgument("unknown argument: " + arg);
        }
    }

    if (options.barsPath.empty() || options.outPath.empty()) {
        return mt::invalidArgument("--bars and --out are both required");
    }
    if (options.windowLength < 2) {
        return mt::invalidArgument("--window must be at least 2");
    }
    if (options.paaSegments > options.windowLength) {
        return mt::invalidArgument("--paa cannot exceed --window");
    }
    return options;
}

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

    // --- load ------------------------------------------------------------
    mtbot::app::CsvLoadReport loadReport;
    const auto bars = mtbot::app::loadBarsFromCsv(opt.barsPath, opt.symbol, loadReport);
    if (!bars) {
        std::cerr << "error: " << bars.error().describe() << '\n';
        return 1;
    }
    std::cout << "loaded " << loadReport.barsAccepted << " bars from " << opt.barsPath;
    if (loadReport.linesSkipped > 0) {
        std::cout << " (" << loadReport.linesSkipped << " line(s) skipped; first: "
                  << loadReport.firstSkipReason << ")";
    }
    std::cout << "\n  range: " << bars.value().front().ts << " .. " << bars.value().back().ts
              << '\n';

    // --- channels --------------------------------------------------------
    const mt::features::ChannelRegistry registry =
        mt::features::defaultEngineRegistry(opt.paaSegments);
    std::cout << "registry: " << registry.size() << " channels in "
              << registry.clusters().size() << " clusters, fingerprint 0x" << std::hex
              << registry.fingerprint() << std::dec << '\n';

    const auto channels = mt::features::buildDefaultChannels(bars.value());
    if (!channels) {
        std::cerr << "error: " << channels.error().describe() << '\n';
        return 1;
    }

    // --- calibrate -------------------------------------------------------
    mt::similarity::CalibrationBuildConfig config;
    config.windowLength = opt.windowLength;
    config.sampling.pairCount = opt.pairCount;
    config.sampling.bins = opt.bins;
    config.sampling.seed = opt.seed;

    std::cout << "sampling " << opt.pairCount << " non-overlapping window pairs, window "
              << opt.windowLength << ", " << opt.bins << " bins\n";

    mt::similarity::CalibrationSet calibration;
    const auto report = mt::similarity::buildCalibration(
        registry, channels.value(), config, calibration,
        [&registry](std::size_t done, std::size_t total) {
            std::cout << "  calibrated " << done << '/' << total << ": "
                      << registry[done - 1].id << '\n';
        });
    if (!report) {
        std::cerr << "error: " << report.error().describe() << '\n';
        return 1;
    }

    if (const mt::Status status = calibration.save(opt.outPath); !status.ok()) {
        std::cerr << "error: " << status.error().describe() << '\n';
        return 1;
    }

    std::cout << "\nwrote " << opt.outPath << '\n'
              << "  channels calibrated: " << report.value().channelsCalibrated << '\n'
              << "  pairs used:          " << report.value().pairsUsable << '\n'
              << "  registry fingerprint: 0x" << std::hex << registry.fingerprint() << std::dec
              << '\n';

    // A quick readout of what the calibration says, so the operator can sanity
    // check it rather than trusting a silent binary.
    std::cout << "\nchannel distance quantiles (1% / 50% / 99%):\n";
    for (const auto& spec : registry.specs()) {
        const auto channel = calibration.get(spec.id);
        if (!channel) continue;
        std::cout << "  " << std::left << std::setw(24) << spec.id << std::right << std::fixed
                  << std::setprecision(4) << std::setw(10) << channel.value()->quantile(0.01)
                  << std::setw(10) << channel.value()->quantile(0.50) << std::setw(10)
                  << channel.value()->quantile(0.99) << '\n';
    }

    return 0;
}
