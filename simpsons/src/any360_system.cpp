// Test branch any360-sys-test: every system call through any360::sys (github.com/GaetanVigner/Any360), each group
// behind the any360_sys_groups setting (empty: the runtime answers them all). The settings, the mounts a console
// title expects and the audio hardware are Any360's (any360/host/settings.h); this file only defines the overrides,
// which must live in the executable.
#include "any360/host/settings.h"

// Every system call any360::sys provides: the runtime's unless its group is switched. The video calls are the
// graphics binding's (any360_graphics.cpp).
#include "any360/host/system_exports.inc"
