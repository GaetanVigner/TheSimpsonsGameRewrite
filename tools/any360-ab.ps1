# Launches The Simpsons Game minimized on Any360, once per variant in the order given, $Seconds each, and prints for
# each the log it wrote ("=== <variant> (<log>)": the runs file Any360's tools/frame_lows.py reads), its hangs and
# errors, its last frame-rate lines and the native functions' check (any360_xdk_verify) or measure (any360_xdk_bench)
# if asked for. Not playable (minimized windows get no gamepad input): the game is driven by Any360's input script.
#
# The runs use their own user data folder (<build>\user_data_ab), never the player's (%LOCALAPPDATA%\simpsons): the
# scripted presses start a game from a fresh profile and must not touch the player's saves.
#
# A variant is extra arguments; "@exe:<name> " first runs another executable of the build folder (a "before" build
# kept beside the new one). tools/any360-route.txt goes from the title screen into the first level (360 s).
#
#   $r = (Get-Content tools\any360-route.txt)
#   .\tools\any360-ab.ps1 -Variants @("--any360_xdk_verify=true $r") -Seconds 360 -Affinity 0xFF -Device NVIDIA
# -Affinity: the processors the title may run on, as a mask (0xFF: cores 0-7), when sessions share the machine;
# -Device: a substring of the Vulkan device to use (--any360_gpu_device).
# A variant holding "@rexglue" runs the title as on ReXGlue, the runtime the rewrite started from: the runtime's GPU
# and system calls and the game's own library functions; only any360::sys's input group stays, to play the same
# input script. Its frame report ("the runtime's GPU: ... frame times ... MB private") reads as Any360's in
# tools/frame_lows.py with --after SECONDS. -Fresh empties user_data_ab before each launch (the same menus each time).
#
#   .\tools\any360-ab.ps1 -Variants @($r, "@rexglue $r", $r, "@rexglue $r") -Seconds 420 -Affinity 0xFF -Device NVIDIA -Fresh
param([string[]]$Variants = @(''), [int]$Seconds = 180, [string]$Build = '', [long]$Affinity = 0, [string]$Device = '',
      [string]$GameData = 'F:\The simpsons\recomp\gamedata', [switch]$Fresh, [switch]$Uncapped, [int]$MenuRate = 0,
      [string]$ShotDir = '', [int[]]$ShotAt = @())
$run = 0
$wt = Split-Path $PSScriptRoot -Parent
if (-not $Build) { $Build = Join-Path $wt 'simpsons\out\build\win-amd64-relwithdebinfo' }
$logs = Join-Path $Build 'logs'
$userData = Join-Path $Build 'user_data_ab'
New-Item -ItemType Directory -Force $userData | Out-Null
foreach ($v in $Variants) {
  $before = @(Get-ChildItem $logs -Filter 'simpsons_*.log' -ErrorAction SilentlyContinue | ForEach-Object { $_.Name })
  # Every launch from the same fresh profile, so the scripted presses meet the same menus.
  if ($Fresh) { Remove-Item -Recurse -Force (Join-Path $userData '*') -ErrorAction SilentlyContinue }
  $extra = @($v -split ' ' | Where-Object { $_ -and $_ -notlike '@*' })
  # -Uncapped: no cap on either side (the game's limiter, the vblank it waits on, the present's vsync), so the two
  # runtimes compare at the rate they can give. -MenuRate keeps the menus' own pacing when the route needs it.
  if ($Uncapped) { $extra = @('--vsync=false', '--frame_rate=0', "--menu_frame_rate=$MenuRate") + $extra }
  $env:ANY360_PRESENT_MODE = if ($Uncapped) { 'immediate' } else { $null }
  if ($v -match '(^| )@rexglue( |$)') {
    # ReXGlue as the rewrite started from: the runtime's GPU and system calls, the game's own library functions. Only
    # any360::sys's input group stays on: it plays the input script (the same presses on both sides).
    $arguments = @("--game_data_root=`"$GameData`"", "--user_data_root=`"$userData`"", '--any360_sys_groups=input',
      '--any360_xdk_natives=false') + $extra
    if ($Uncapped) { $arguments += '--unlocked_vblank_hz=1000' }
  } else {
    $arguments = @("--game_data_root=`"$GameData`"", "--user_data_root=`"$userData`"", '--any360_gpu=true',
      '--any360_sys_groups=stateless,files,objects,threads,memory,xam,input,audio') + $extra
    if ($Device) { $arguments += "--any360_gpu_device=$Device" }
    if ($Uncapped) { $arguments += '--any360_gpu_vblank_hz=1000' }
  }
  $shot = $null
  if ($ShotDir -and $v -notmatch '@rexglue') {
    New-Item -ItemType Directory -Force $ShotDir | Out-Null
    $shot = Join-Path $ShotDir "run$run.png"
    $arguments += "--any360_gpu_screenshot=`"$shot`""
  }
  $exe = 'simpsons.exe'
  if ($v -match '^@exe:(\S+)') { $exe = $Matches[1] }
  $p = Start-Process -FilePath (Join-Path $Build $exe) -ArgumentList $arguments -WorkingDirectory $Build -WindowStyle Minimized -PassThru
  if ($Affinity) { $p.ProcessorAffinity = [IntPtr]$Affinity }  # every thread, those already started included
  for ($i = 0; $i -lt $Seconds -and -not $p.HasExited; $i++) {
    Start-Sleep -Seconds 1
    # -ShotDir/-ShotAt: Any360's frame screenshots (the renderer's frame read back, never the screen) at those seconds.
    if ($shot -and $ShotAt -contains $i) {
      Remove-Item $shot -ErrorAction SilentlyContinue
      New-Item -ItemType File "$shot.go" -Force | Out-Null
      for ($k = 0; $k -lt 10 -and -not (Test-Path $shot); $k++) { Start-Sleep -Milliseconds 300 }
      Start-Sleep -Milliseconds 300
      if (Test-Path $shot) { Move-Item $shot ($shot -replace '\.png$', ('_t{0:000}.png' -f $i)) -Force } else { "no screenshot at $i s" }
    }
  }
  $run++
  if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force } else { "exited (code $($p.ExitCode))" }
  Start-Sleep -Seconds 2
  $log = Get-ChildItem $logs -Filter 'simpsons_*.log' | Where-Object { $before -notcontains $_.Name } | Sort-Object LastWriteTime | Select-Object -Last 1
  "=== $v ($($log.Name))"
  $l = Get-Content $log.FullName
  "hangs: $(($l | Select-String '\[HANG\]').Count); errors: $(($l | Select-String '\[error\]').Count)"
  $l | Select-String "Any360: [0-9.]+ FPS; per frame|Any360: the runtime's GPU: [0-9.]+ FPS" | Select-Object -Last 2 | ForEach-Object { $x = $_.Line; $i0 = $x.IndexOf('Any360:'); $x.Substring($i0, [Math]::Min(200, $x.Length - $i0)) }
  # The frame rate along the route (one figure a period), to see where the menus end and the level starts.
  $rates = $l | Select-String "Any360: ([0-9.]+) FPS with the diagnostics off|Any360: the runtime's GPU: ([0-9.]+) FPS" | ForEach-Object { $m = $_.Matches[0]; if ($m.Groups[1].Success) { $m.Groups[1].Value } else { $m.Groups[2].Value } }
  "FPS along the route: $($rates -join ' ')"
  $l | Select-String 'calls verified' | Select-Object -Last 1 | ForEach-Object { $x = $_.Line; $x.Substring($x.IndexOf('Any360 xdk')) }
  $l | Select-String 'Any360 xdk bench: .*calls, own code' | Select-Object -Last 12 | ForEach-Object { $x = $_.Line; $x.Substring($x.IndexOf('Any360 xdk'), [Math]::Min(220, $x.Length - $x.IndexOf('Any360 xdk'))) }
}
# Only this script's launches (their user data folder), never another session's or the player's.
Get-CimInstance Win32_Process -Filter "name like 'simpsons%.exe'" | Where-Object { $_.CommandLine -like "*user_data_ab*" -and $_.CommandLine -like "*$Build*" } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force; 'killed stray' }
