// Native memset and memcpy for the game's C runtime.
//
// The game's own memset (sub_82A3C440) and copies run as recompiled loops of
// byte-swapped loads and stores, which made them several percent of the main
// thread. A byte fill or copy is the same in either byte order, so these do
// the work with the host's routines. All take r3 = destination, r4 = fill byte
// or source, r5 = size, and leave the destination in r3:
// - sub_82A3CD80, memcpy, which also leaves it at r1 - 8, below the stack
//   pointer, and sub_82A3D1D8, a simpler one;
// - sub_8270A590, a copy of whole 64-byte blocks (the shader constants the
//   renderer writes into command buffers);
// - sub_82B74110, the XDK's copy for write-combined memory (vector stores,
//   cache lines zeroed ahead of being overwritten), whose tail copies bytes
//   while subtracting a global that is 1.
//
// The game's copies run forward. For a destination above an overlapping
// source that repeats the copied bytes, which the host's memcpy and memmove
// don't, so such a copy keeps the original routine, as does anything else
// outside the cases above.
//
// In a build with Any360 (SIMPSONS_ANY360), memset, memcpy, the simpler copy,
// memmove (sub_82A3E2B0) and the XDK's vector copy (sub_82B74110) are Any360's
// native functions, shared by every title (any360::xdk: unit tests, a
// benchmark and the any360_xdk_verify / any360_xdk_bench settings, Any360's
// docs/NATIVE_FUNCTIONS.md); the code here for them is only what a build
// without Any360 runs. The 64-byte block copy is this title's own code, not
// a library's: it stays here.

#include "guest_memory.h"

#include <cstdint>
#include <cstring>

#include <rex/ppc.h>

#if SIMPSONS_ANY360
#include "any360/host/xdk_natives.h"
#endif

REX_EXTERN(__imp__sub_82A3C440);
REX_EXTERN(sub_82A3C440);
REX_EXTERN(__imp__sub_82A3CD80);
REX_EXTERN(sub_82A3CD80);
REX_EXTERN(__imp__sub_82A3D1D8);
REX_EXTERN(sub_82A3D1D8);
REX_EXTERN(__imp__sub_8270A590);
REX_EXTERN(sub_8270A590);
REX_EXTERN(__imp__sub_82B74110);
REX_EXTERN(sub_82B74110);

namespace {

using simpsons::GuestToHost;
using simpsons::LoadGuestU32;

// The step sub_82B74110's byte loop subtracts from the count.
constexpr uint32_t kCopyByteStep = 0x82CFE4C0;

// A forward copy of size bytes from source to destination gives the same
// result as memmove unless the destination starts inside the source.
bool ForwardCopyIsPlain(uint32_t destination, uint32_t source, uint32_t size) {
  return destination <= source || destination - source >= size;
}

// Whether [address, address + size) stays within one side of the physical
// address boundary, so that one host pointer covers it on every platform.
bool OneHostRange(uint32_t address, uint32_t size) {
  const uint64_t end = uint64_t(address) + size;
  return end <= 0x100000000ull && (address >= 0xE0000000u || end <= 0xE0000000ull);
}

}  // namespace

#if SIMPSONS_ANY360

ANY360_XDK_BIND(82A3C440, CrtMemset)
ANY360_XDK_BIND(82A3CD80, CrtMemcpy)
ANY360_XDK_BIND(82A3D1D8, CrtCopyForward)
ANY360_XDK_BIND(82A3E2B0, CrtMemmove)
// The XDK's vector copy; kCopyByteStep is the global its byte loop subtracts from the count (1).
ANY360_XDK_BIND_WITH(82B74110, XMemCpy, kCopyByteStep)
// The C runtime's string functions. wcslen (sub_82A3FED0), wcscpy (sub_82A41268), wcsncpy (sub_82CADFC8), tolower
// (sub_826BE740) and toupper (sub_827418C0) have the same replacements in Any360 and are not bound: no run of the
// game has called them with both sides compared.
ANY360_XDK_BIND(82A3FE90, CrtStrchr)
ANY360_XDK_BIND(82A3EF20, CrtStrncmp)
ANY360_XDK_BIND(82A3D280, CrtStrncpy)
ANY360_XDK_BIND(82A428E0, CrtStrrchr)
ANY360_XDK_BIND(82A3F030, CrtStrncat)
ANY360_XDK_BIND(82A419A0, CrtStricmp)
ANY360_XDK_BIND(82A3EFB0, CrtStrstr)
// The C runtime's helpers on a double's bits: _finite, _get_exp and _set_exp (the last two are in the image
// twice). _isnan (sub_82A42EA8), _sptype (sub_82A48BA8) and _add_exp (sub_82A48B48) have their replacements in
// Any360 too and are not bound: no run of the game has called them with both sides compared.
ANY360_XDK_BIND(82A42E88, CrtFinite)
ANY360_XDK_BIND(82A43B38, CrtGetExp)
ANY360_XDK_BIND(82A48B30, CrtGetExp)
ANY360_XDK_BIND(82A43B50, CrtSetExp)
ANY360_XDK_BIND(82A48B08, CrtSetExp)
// mbtowc (sub_82A5A5F0) has its replacement in Any360 too and is not bound: no run of the game has called it with
// both sides compared. The strlen and strcmp built as loops of bytes (sub_82233B00, sub_823896A8) are Havok's own:
// middleware stays as it is, they are not bound.
ANY360_XDK_BIND(82CADF80, CrtWcscmp)
// XAPI's GetLastError (sub_82433BD0) and SetLastError (sub_82433B80) have their replacements in Any360, checked here
// with both sides compared (4,819 and 50,734 calls, none different), and are not bound: five instructions each,
// they gain nothing as host routines.
// XAudio's fill of a block's buffer with its value (sub_82C6E9A8, sub_82C6E870, sub_82C6EA10, sub_82C6E940,
// sub_82C6EA78: 256 to 4096 floats) has its replacement in Any360 too and is not bound: no run of the game has called
// it with both sides compared.

#else

REX_FUNC(sub_82A3C440) {
  const uint32_t destination = ctx.r3.u32;
  const uint32_t size = ctx.r5.u32;
  if (!OneHostRange(destination, size)) {
    __imp__sub_82A3C440(ctx, base);
    return;
  }
  std::memset(GuestToHost(base, destination), int(ctx.r4.u32 & 0xFF), size);
}

REX_FUNC(sub_82A3CD80) {
  const uint32_t destination = ctx.r3.u32;
  const uint32_t source = ctx.r4.u32;
  const uint32_t size = ctx.r5.u32;
  if (!ForwardCopyIsPlain(destination, source, size) || !OneHostRange(destination, size) ||
      !OneHostRange(source, size)) {
    __imp__sub_82A3CD80(ctx, base);
    return;
  }
  // std r3,-8(r1): the register's 64 bits, not the address alone.
  const uint64_t saved = std::byteswap(ctx.r3.u64);
  std::memcpy(GuestToHost(base, ctx.r1.u32 - 8), &saved, sizeof(saved));
  std::memmove(GuestToHost(base, destination), GuestToHost(base, source), size);
}

REX_FUNC(sub_82A3D1D8) {
  const uint32_t destination = ctx.r3.u32;
  const uint32_t source = ctx.r4.u32;
  const uint32_t size = ctx.r5.u32;
  if (!ForwardCopyIsPlain(destination, source, size) || !OneHostRange(destination, size) ||
      !OneHostRange(source, size)) {
    __imp__sub_82A3D1D8(ctx, base);
    return;
  }
  std::memmove(GuestToHost(base, destination), GuestToHost(base, source), size);
}

#endif  // SIMPSONS_ANY360

REX_FUNC(sub_8270A590) {
  const uint32_t destination = ctx.r3.u32;
  const uint32_t source = ctx.r4.u32;
  const uint32_t size = ctx.r5.u32;
  if ((size & 63) || !ForwardCopyIsPlain(destination, source, size) ||
      !OneHostRange(destination, size) || !OneHostRange(source, size)) {
    __imp__sub_8270A590(ctx, base);
    return;
  }
  std::memmove(GuestToHost(base, destination), GuestToHost(base, source), size);
}

#if !SIMPSONS_ANY360

REX_FUNC(sub_82B74110) {
  const uint32_t destination = ctx.r3.u32;
  const uint32_t source = ctx.r4.u32;
  const uint32_t size = ctx.r5.u32;
  // The routine zeroes the destination's cache lines ahead of writing them: two ranges that overlap are its own
  // business (the zeroed line can hold source bytes it has not read yet).
  const bool apart = destination >= source ? destination - source >= size : source - destination >= size;
  if (LoadGuestU32(base, kCopyByteStep) != 1 || !apart || !OneHostRange(destination, size) ||
      !OneHostRange(source, size)) {
    __imp__sub_82B74110(ctx, base);
    return;
  }
  std::memmove(GuestToHost(base, destination), GuestToHost(base, source), size);
}

#endif  // !SIMPSONS_ANY360
