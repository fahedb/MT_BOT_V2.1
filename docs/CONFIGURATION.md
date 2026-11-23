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

## Tests

```powershell
ctest --preset vs2022-debug          # all tests
.\build\vs2022-debug\bin\Debug\mt_tests.exe --gtest_filter=BaselineV0.*
```

Two groups:

* **`BaselineV0.*`** — the pre-refactor scoring math must still reproduce
  `tests/golden/baseline_v0/`. Regenerate those files only on purpose:
  `mt_gen_baseline.exe`, then record what moved and why in the plan's progress
  log.
* **`Finding_*.*`** — characterisation tests that pin down the defects listed
  in section 4 of the plan. They assert today's behaviour, including where it
  is wrong, and get rewritten by the phase that fixes each finding.

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
