# Configuration and build

Phase 0 of [REFACTOR_PLAN.md](REFACTOR_PLAN.md). This file covers how to build
and where machine-specific settings now come from. The full typed engine
configuration (`config/engine.default.toml` and the run manifest) arrives in
Phase 6; until then the legacy SQL config table remains the engine's source of
settings.

## Build

Requirements:

* Visual Studio 2022 or newer (MSVC v143+), Windows SDK 10
* CMake 3.24+ — the copy bundled with Visual Studio works
  (`…\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`)
* vcpkg, with `nlohmann-json` and `gtest` for the `x64-windows` triplet
* CUDA Toolkit 12.x, only if you turn the GPU kernels on

```powershell
cmake --preset vs2022-debug
cmake --build --preset vs2022-debug
ctest --preset vs2022-debug
```

vcpkg is located automatically: `-DMTBOT_VCPKG_ROOT=...`, else `%VCPKG_ROOT%`,
else `C:\vcpkg`. The presets set `VCPKG_MANIFEST_MODE=OFF` so an existing
classic-mode vcpkg installation is reused (fast, no network). CI runs in
manifest mode and installs from `vcpkg.json` instead.

**Keep the checkout path short.** The Visual Studio generator writes its
compiler-probe projects under `build/<preset>/CMakeFiles/CMakeScratch/...`, and
with a deep source path those filenames exceed Windows' 260-character limit.
The symptom is a confusing configure failure during the compiler check:
`MSB6003: The specified task executable "CL.exe" could not be run.
System.IO.DirectoryNotFoundException`. Something like `C:\src\mtbot` is safe;
a clone nested a hundred characters deep is not. Enabling long paths
(`git config --global core.longpaths true` plus the Windows
`LongPathsEnabled` policy) or using the `ninja-debug` preset also avoids it.

### Building the legacy engine

```powershell
cmake --preset vs2022-legacy
cmake --build --preset vs2022-legacy
```

This adds `mt_legacy_core`: all 110 pre-refactor MTCore + HTTPCore sources as
one static library (~244 MB in Debug). It needs the CUDA Toolkit headers
(`CudaMemoryController.h` includes `<cuda_runtime.h>` even on the CPU path) and
cairo (for `MTChart.cpp`; without it that one file is skipped and PNG export is
disabled).

**Build it serially.** The preset sets `jobs: 1` on purpose. The legacy sources
pull in `robin_hood.h` plus heavy standard headers, and a parallel build of all
110 translation units can exhaust the compiler heap — `C1060: compiler is out
of heap space` — even on a 64 GB machine, if other applications are holding the
system commit charge. If `C1060` appears anyway, close memory-hungry
applications and check the headroom:

```powershell
Get-CimInstance Win32_OperatingSystem | Select-Object @{N='FreeVirtualMB';E={[math]::Round($_.FreeVirtualMemory/1KB)}}
```

Plenty of free *physical* memory with little free *virtual* memory still fails.
Finding E17 in the plan; the Phase 1 header de-duplication and Phase 2's
smaller translation units both reduce it.

### Build options

| Option | Default | Meaning |
|---|---|---|
| `MTBOT_BUILD_TESTS` | `ON` | Build and register the test suite |
| `MTBOT_BUILD_LEGACY_FULL` | `OFF` | Build `mt_legacy_core`, the whole pre-refactor engine. `ON` in the `vs2022-legacy` preset |
| `MTBOT_ENABLE_CUDA` | `OFF` | Build the CUDA scoring kernels |
| `MTBOT_WARNINGS_AS_ERRORS` | `OFF` | `/WX` for code written during the refactor; CI sets this `ON` |

The legacy Visual Studio solutions (`MTScanner.sln` and the rest) are still
present and untouched. They are retired in Phase 6, not before.

## The tools

Both read plain CSV — `timestamp,open,high,low,close[,volume]`, with the
timestamp either epoch milliseconds or `YYYY-MM-DD HH:MM:SS` — so the engine
can be calibrated and evaluated on real history before any of the live
plumbing is rewired. Exporting from the existing tables is a one-line query.

No data to hand? Generate some:

```powershell
python tools\make_synthetic_bars.py --out bars.csv --bars 20000
```

### mtcalibrate

Builds the calibration artefact, without which there is no comparable
similarity and therefore no meaningful score.

```powershell
.\build\vs2022-release\bin\Release\mtcalibrate.exe --bars bars.csv --out cal.bin --symbol XAUUSD --window 48 --paa 12 --pairs 200000
```

It prints each channel's distance quantiles at 1% / 50% / 99%, so the
calibration can be sanity-checked rather than trusted as a silent binary.

**Fit it on training data only.** A calibration whose span includes the period
you then evaluate on is a look-ahead: the null distribution already knows about
the future (finding S11).

### mtbacktest

```powershell
.\build\vs2022-release\bin\Release\mtbacktest.exe --bars bars.csv --calibration cal.bin --window 48 --paa 12 --cost 20 --min-score 70 --step 20
```

`--window` and `--paa` must match what `mtcalibrate` used, or the calibration is
refused — standardisation changes invalidate it, and the registry fingerprint
catches that.

It evaluates only bars after the calibration span, subtracts `--cost` on every
trade, and reports *why* it declined each untraded bar. A run that reports
`trades: 0` is a working engine saying nothing in history resembles the
present — which is the behaviour the previous design could not produce.

It also prints what it is not. A single split with no baselines and no trial
accounting is not evidence; section 8 of the plan says what would be.

## Tests

```powershell
ctest --preset vs2022-debug          # all tests
.\build\vs2022-debug\bin\Debug\mt_tests.exe --gtest_filter=BaselineV0.*
```

```powershell
ctest --preset vs2022-debug -LE integration   # 158 unit tests, ~36 s
ctest --preset vs2022-debug -L integration    # 10 integration tests, ~53 s
```

Groups:

* **`BaselineV0.*`** — the pre-refactor scoring math must still reproduce
  `tests/golden/baseline_v0/`. Regenerate those files only on purpose:
  `mt_gen_baseline.exe`, then record what moved and why in the plan's progress
  log.
* **`Finding_*.*`** — characterisation tests that pin down the defects listed
  in section 4 of the plan. They assert today's behaviour, including where it
  is wrong, and get rewritten by the phase that fixes each finding.
* **`ScoringAcceptance.*`** — the contract from section 6.9 for the new
  scoring: identity, symmetry, level and scale invariance, monotonicity under
  noise, bounded outlier influence, calibration uniformity, and refusal of a
  mismatched calibration. If one of these fails, the scoring system is no
  longer the one the plan specified.
* **`HnswSearch.RecallAgainstExactMeetsTheGate`** — the ANN index must find at
  least 95% of what brute force finds. This is what makes the retrieval swap
  safe rather than hopeful.
* **`EngineEndToEnd.*`** (integration) — the whole chain on a planted motif,
  including that it stands aside when there is nothing to find.

## Environment variables

Replacing hardcoded machine-specific values (plan finding E12).

| Variable | Used by | Default if unset |
|---|---|---|
| `MTBOT_SQL_CONN` | every Python script that uses SQLAlchemy (`PYTH/**`) | `mssql+pyodbc://localhost\sqlexpress/Prices?driver=ODBC+Driver+17+for+SQL+Server&trusted_connection=yes` |
| `MTBOT_ODBC_CONN` | the raw-pyodbc injectors in `PYTH/TradeDeccissionML` | `DRIVER={SQL Server};SERVER=localhost\sqlexpress;DATABASE=Prices;Trusted_Connection=yes;` |
| `VCPKG_ROOT` | the CMake build | `C:\vcpkg` |

The defaults match what was hardcoded before, so an existing developer machine
keeps working with nothing set.

```powershell
$env:MTBOT_SQL_CONN = "mssql+pyodbc://myhost\sqlexpress/Prices?driver=ODBC+Driver+18+for+SQL+Server&trusted_connection=yes&TrustServerCertificate=yes"
```

Note the two formats are genuinely different (SQLAlchemy URL vs raw ODBC), which
is why there are two variables rather than one. Phase 7 consolidates the Python
trees and this becomes a single settings module.

## MT5 deployment

`mql5/Feeder/PriceFeeder.mq5` previously imported the DLL by absolute path out
of one developer's user profile. It now imports it by name:

```mql5
#import "MTDll.dll"
```

MQL5 requires a compile-time literal here, so this cannot be a parameter. The
DLL has to be where MetaTrader looks for it:

1. build `MTDll` (x64, matching your terminal's bitness);
2. copy or hard-link the output to
   `<terminal data folder>\MQL5\Libraries\MTDll.dll`
   (File → Open Data Folder in MetaTrader);
3. enable *Allow DLL imports* in the terminal's Expert Advisors options.

A known inconsistency, left alone deliberately: the EA includes its helpers from
`../Include/` while this repository stores them in `mql5/Libs/`. That reflects
how the files are laid out inside the MT5 data folder; it is tidied in Phase 6
together with the rest of the bridge, not now, because changing it would break a
working terminal install.

## What is deliberately NOT configurable yet

The engine's numeric parameters (window length, thresholds, weights, filter
strings) still come from the legacy SQL config table through
`MTCore/MTSimulator_Configurations`. Section 4 of the plan records why that is a
problem — dead weight keys (B8), a `threshold` that actually means "fraction
kept" (B9), hardcoded constants in algorithm code (`COMAPRE_WEIGHT`, the `0.98`
and `20` in `PatternGod.cpp`). Fixing it is Phase 6, and the replacement is a
single typed config plus a hashed run manifest.
