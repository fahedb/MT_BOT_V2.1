# third_party

Vendored, unmodified upstream libraries. **Do not edit these files.** If one
needs changing, upgrade it deliberately and record the version here.

| Library | Version | Upstream | Used by |
|---|---|---|---|
| `robin_hood/robin_hood.h` | 3.11.5 | https://github.com/martinus/robin-hood-hashing | hash maps in `MTSimulator_Cache`, `_Configurations`, `_TimeLines`, `_Statistics`, `TradeSimulator`, `MTAnalyzer` |
| `cpp-httplib/httplib.h` | 0.18.3 | https://github.com/yhirose/cpp-httplib | `HTTPCore` server/client, `MTSimulator_Logger`'s HTTP log sink |

## Why there are forwarding headers

Finding E5 in [../docs/REFACTOR_PLAN.md](../docs/REFACTOR_PLAN.md): each of
these headers had been copied next to every module that used it —
`robin_hood.h` five times and `httplib.h` twice, 33,236 duplicated lines in
total, all byte-identical (verified by checksum before consolidation).

The legacy `.vcxproj` files do not put any shared directory on their include
paths; each project resolved `"robin_hood.h"` or `"httplib.h"` out of its own
module folder. So the old locations now hold short forwarding headers that
`#include` the single real copy here. That keeps every existing project
building without touching project files that are retired in Phase 9.

New code includes these directly — `third_party/robin_hood` and
`third_party/cpp-httplib` are on the CMake include path. The forwarders go away
with the modules that need them.

## Preferring the standard library

`robin_hood` predates the parts of the refactor that care about it. In `libs/`,
reach for `std::unordered_map` unless a measurement says otherwise; keep
`robin_hood` only where profiling shows it earns its place (the score-result
and polynomial caches are the plausible candidates, and Phase 4 removes much of
that caching anyway).
