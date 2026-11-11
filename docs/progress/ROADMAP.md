# The Simpsons Game: roadmap

Generated on 2026-11-11 with [PROGRESS.md](PROGRESS.md). The rewrite is done when nothing of the Xbox 360 or of ReXGlue's runtime is left under the game: every system call on `any360::sys`, graphics on `any360::gfx`, Any360 as the host, and the game's code as portable C++. Steps are in working order; each lists what it still needs.

## 1. System calls on any360::sys

All 317 imports run on `any360::sys` (all eight groups, opt-in on `main` (built with `SIMPSONS_ANY360_DIR`): `--any360_sys_groups=stateless,files,objects,threads,memory,xam,input,audio`; levels, saves, sound and gamepad checked in game).

Still to do: make it the default build (today it is opt-in), then remove the ReXGlue kernel from the link.

## 2. Graphics on any360::gfx

The title draws through `any360::gfx`: opt-in on `main` (built with `SIMPSONS_ANY360_DIR`): `--any360_gpu=true`; 60 FPS in gameplay and 30 on menus and loading screens (the game's own pacing), as with the runtime's GPU.

Still to do: make it the default renderer (today ReXGlue's Xenos emulation is), then drop the emulated GPU from the build.

- Anti-aliasing: host MSAA smooths the game's data buffers along with its colours (false outlines), so the title runs without it (`--any360_gpu_msaa=1`); it needs MSAA that tells data from colour, or supersampling.

GPU features `any360::gfx` does not handle yet (40 of 323; none stops this title today, each is one a title may need):

| Family | Features |
|---|---|
| command processor | MEM_WRITE_CNTR, COND_EXEC, EVENT_WRITE_CFL, DRAW_INDX_2_BIN, SET_STATE, SET_BIN_BASE_OFFSET, IM_STORE |
| primitive types | kTriangleWithWFlags, kExplicitMajorModeForceStart, k2DCopyRectListV1, k2DCopyRectListV2, k2DCopyRectListV3, k2DFillRectList, k2DLineStrip, k2DTriStrip |
| render features | tessellation |
| texture formats | k_1_REVERSE, k_1, k_8_B, k_16_16_EDRAM, k_8_8_8_8_A, k_16_16_16_16_EDRAM, k_32, k_32_32, k_32_32_32_32, k_32_AS_8, k_32_AS_8_8, k_16_MPEG, k_16_16_MPEG, k_8_INTERLACED, k_32_AS_8_INTERLACED, k_32_AS_8_8_INTERLACED, k_16_INTERLACED, k_16_MPEG_INTERLACED, k_16_16_MPEG_INTERLACED, k_32_32_32_FLOAT, k_DXT3A_AS_1_1_1_1, k_8_8_8_8_GAMMA_EDRAM, k_2_10_10_10_FLOAT_EDRAM |
| vertex formats | kUndefined |

## 3. Any360 as the host

Shared by every title (Any360's own roadmap); the title follows once they land.

| Any360 step | What | State |
|---|---|---|
| 11 | third and fourth titles (Test Drive Unlimited, Sonic 06) | partial: Test Drive Unlimited boots to its title screen with all eight groups on `any360::sys` and no call reaching the runtime; gameplay, saves and sound not checked yet; Sonic 06 not started |
| 12 | paths no title took yet: `XMA*` decoding, STFS packages, several users, guide UIs, Live offline, xbdm | partial: 602 of 2,914 exports native with scenario tests, none of the 2,294 reference stubs sorted |
| 13 | Any360's own host: XEX loader, address space, threads, guest dispatch, MMIO, devices | not started |
| 14 | the platform layer: window, input, audio, presenter on Windows and Linux (Steam Deck) | partial: SDL3 audio output only |
| 15 | graphics through `any360::gfx` only, no `rexgpu-xenos` | partial: three titles draw through `any360::gfx`, the GPU session's plan in [docs/GFX.md](https://github.com/GaetanVigner/Any360/blob/main/docs/GFX.md) |
| 16 | porting a new title: a converter served from GitHub (as AnyPS5) and built into the cross-platform launcher, from a XEX to a running build with no title-specific code in Any360 | not started |

## 4. Library and middleware code

SDK code linked into the executable is not decompiled: Any360 provides the same service natively and the whole range goes.

| Library range | Functions | Instructions | Replaced | Verified in the sandbox | Left |
|---|---|---|---|---|---|
| XDK D3DX and XGraphics (shader compiler) | 10,871 | 676,089 | 0 | 0 | 10,871 |
| C runtime (CRT) | 1,091 | 54,874 | 11 | 0 | 1,080 |
| XDK audio (XAudio, XMA) | 1,430 | 54,273 | 0 | 0 | 1,430 |
| XDK D3D runtime | 1,636 | 53,387 | 2 | 0 | 1,634 |
| XDK Xbox Live and voice (XOnline, XHV) | 756 | 30,487 | 0 | 0 | 756 |
| XAPI (Win32 layer over the kernel and XAM) | 348 | 15,885 | 2 | 0 | 346 |
| kernel import thunks | 96 | 220 | 0 | 0 | 96 |

Third-party middleware the game links (physics, AI, scripting, video and the like) is neither the game's own code nor something Any360 provides. It is not rewritten function by function: it stays recompiled, or is replaced by the vendor's library for the target platform (or an open implementation) once the game's code around it is portable.

| Middleware | Functions | Instructions |
|---|---|---|
| Havok Physics | 6,638 | 277,751 |
| Lua | 883 | 34,174 |

## 5. Game code as portable C++

58,102 of 58,125 game functions are still recompiled PowerPC. The list, function by function, is [functions.csv](functions.csv) (state `recompiled` or `verified`). By module, largest first:

| Module | Functions left | Instructions left | Rewritten | Verified |
|---|---|---|---|---|
| code 822xxxxx | 10,170 | 208,106 | 0 | 0 |
| code 829xxxxx | 9,606 | 319,997 | 4 | 0 |
| code 827xxxxx | 8,998 | 269,968 | 5 | 0 |
| code 823xxxxx | 8,299 | 263,107 | 3 | 0 |
| code 828xxxxx | 8,142 | 256,017 | 1 | 0 |
| code 826xxxxx | 4,745 | 120,596 | 8 | 0 |
| code 82Cxxxxx | 3,730 | 92,981 | 1 | 0 |
| code 82Axxxxx | 2,705 | 106,094 | 0 | 0 |
| code 824xxxxx | 945 | 51,037 | 0 | 0 |
| code 82Bxxxxx | 762 | 51,575 | 1 | 0 |

Order of work: symbol recovery and library recognition first (they shrink this list), then leaf functions (no calls: each is checked alone), then their callers, module by module; every rewrite is bound over the recompiled function and proven equal in the guest sandbox before it replaces it (Any360's `pipeline/any360.py <title> select | decompile | bind | verify`).

## 6. This title

- Frame rates above 60: the game clock, physics and menus keep their speed; the camera does not yet (see known problems).
- Upstream's own roadmap stays valid on top: widescreen, controller prompts, restored content, mod support, Android.

## Known problems

- Above 60 FPS the camera (right stick) barely turns: the title's camera logic, not the input path (every other input is right).
- Loading screens and the heaviest scenes drop to 21-25 FPS: bound by the game's own CPU thread, not by the renderer (1-3 ms a frame).
- On some launches the menus and loading screens run at 22 FPS instead of 30 with the window shown (7 launches of 11; never minimized; levels keep 60): the renderer's thread waits in the presentation of each frame. Not looked into yet.
