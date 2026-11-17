// The gameplay camera's right stick above 60 fps.
//
// Two camera behaviours turn the camera with the right stick, once per frame: sub_8295C9C0 and sub_82960870 (both
// virtual). Each scales the stick into an angle for this frame, proportional to the frame's time, keeps an axis
// only when its angle passes a threshold (0.02 rad: the float at 0x82CF8EDC for the first, 0x82CF8F80 for the
// second; neither is read anywhere else) and adds what it keeps to the camera's angles. At 60 fps a full deflection
// gives about 0.06 rad a frame and passes; above 60 fps the angle a frame shrinks with the frame time and falls
// under the threshold, so the stick is dropped (measured at about 236 fps: 1 or 2 rotations a second out of about
// 220 deflected reads, 0.04 rad/s instead of 2.5).
//
// The threshold is a rate in disguise (an angle per 60 Hz frame). While a frame is shorter than a 59.94 Hz vblank,
// each behaviour runs with its threshold scaled by the frame's share of a vblank, then the threshold is put back:
// the same angular speed passes at any frame rate. At 60 fps and below nothing changes.

#include "guest_memory.h"

#include <cstdint>

#include <rex/cvar.h>
#include <rex/ppc.h>

REXCVAR_DEFINE_BOOL(camera_stick_any_rate, true, "Camera",
                    "Above 60 fps, scale the camera's right-stick thresholds with the frame time so the stick still "
                    "turns the camera");

REX_EXTERN(__imp__sub_8295C9C0);
REX_EXTERN(sub_8295C9C0);
REX_EXTERN(__imp__sub_82960870);
REX_EXTERN(sub_82960870);

namespace {

using simpsons::LoadGuestFloat;
using simpsons::StoreGuestFloat;

// The game clock's frame time in seconds (game_clock.cpp: real time while frames are fractional).
constexpr uint32_t kFrameDt = 0x82CED748;
constexpr float kVblank = 1.0f / 59.94f;

template <void (*Original)(PPCContext&, uint8_t*)>
void WithFrameThreshold(PPCContext& ctx, uint8_t* base, uint32_t threshold) {
  const float dt = LoadGuestFloat(base, kFrameDt);
  if (!REXCVAR_GET(camera_stick_any_rate) || !(dt > 0.0f && dt < kVblank)) {
    Original(ctx, base);
    return;
  }
  const float original = LoadGuestFloat(base, threshold);
  StoreGuestFloat(base, threshold, original * (dt / kVblank));
  Original(ctx, base);
  StoreGuestFloat(base, threshold, original);
}

}  // namespace

REX_FUNC(sub_8295C9C0) { WithFrameThreshold<__imp__sub_8295C9C0>(ctx, base, 0x82CF8EDC); }

REX_FUNC(sub_82960870) { WithFrameThreshold<__imp__sub_82960870>(ctx, base, 0x82CF8F80); }
