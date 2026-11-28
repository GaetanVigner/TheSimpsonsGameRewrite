# Any360 against ReXGlue, 2026-11-26

The Simpsons Game on Any360 `2f9a9d1` (all eight `any360::sys` groups, `any360::gfx`, the `any360::xdk` natives)
against the same build on ReXGlue's own runtime and GPU, five launches each, alternating, the same input script
(`tools/any360-route.txt`: menus, the opening cinematic, then the first level, Start every 60 s).

- Machine: RTX 5070 Ti, 420 s a launch, fresh user data. The launch script set the mask 0xFF (`-Affinity 0xFF`), but the runtime reopens the process to every processor at start: both variants ran on all 16 logical processors.
- Uncapped on both sides: `--vsync=false --frame_rate=0 --menu_frame_rate=0`; ReXGlue `--unlocked_vblank_hz=1000`;
  Any360 `--any360_gpu_vblank_hz=1000` and `ANY360_PRESENT_MODE=immediate`. Neither variant sits on a cap (both run
  at 700-900 FPS in the level).
- The ReXGlue variant keeps `--any360_sys_groups=input` only, because the input script needs it; no `--any360_gpu`,
  `--any360_xdk_natives=false`. Its frames are counted by Any360's runtime-GPU line (VdSwap calls).
- Measured: the periods logged 150 s or more after launch (the level), `tools/frame_lows.py --after 150` (Any360).

Files:

- `ab-output.txt`: the launch script's output (`tools/any360-ab.ps1 -Fresh -Uncapped`), with the in-game check launch first.
- `runs.txt`: the ten measured launches.
- `logs/`: the Any360 launches' logs whole; the ReXGlue launches' logs (17 MB each, rotated) cut to their first line
  and every `Any360` / error line, which is all `frame_lows.py` reads.
- `frame_lows.txt`: `python -I tools/frame_lows.py runs.txt logs --after 150`.

Result (steady periods, means over launches):

| | Any360 | ReXGlue | z |
|---|---|---|---|
| FPS | 815.7 (median 809.2) | 727.3 (median 731.9) | +16.41 |
| 10% low | 725.9 | 659.7 | +15.67 |
| 1% low | 598.0 | 404.4 | +16.41 |
| Dip 1% low | 544.3 | 333.2 | +2.61 |
| Slowest frame in the dip | 7.4 ms | 49.7 ms | -2.61 |
| Private memory | 2800 MB (max 2842) | 1703 MB (max 1729) | |

Any360 is faster on every measure, and its 1% lows are 48% higher; it takes about 1.1 GB more private memory, not yet
broken down: the next thing to look at for small-RAM targets.
