// ---------------------------------------------------------------------------
// Forwarding header. The real cpp-httplib 0.18.3 lives in third_party/.
//
// Finding E5: this header existed as 2 byte-identical copies, one next
// to each module that used it (20,516 duplicated lines in total). The legacy
// .vcxproj files do not put the canonical directory on their include paths,
// so they resolve "httplib.h" from the module directory. Rather than edit every
// project file - they are retired in Phase 9 - each old location now forwards
// to the single real copy.
//
// Do not edit the file this includes: it is vendored, unmodified upstream.
// See third_party/README.md.
// ---------------------------------------------------------------------------
#pragma once

#include "../../third_party/cpp-httplib/httplib.h"
