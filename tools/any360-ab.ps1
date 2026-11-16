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
param([string[]]$Variants = @(''), [int]$Seconds = 180, [string]$Build = '', [long]$Affinity = 0, [string]$Device = '',
      [string]$GameData = 'F:\The simpsons\recomp\gamedata')
$wt = Split-Path $PSScriptRoot -Parent
if (-not $Build) { $Build = Join-Path $wt 'simpsons\out\build\win-amd64-relwithdebinfo' }
$logs = Join-Path $Build 'logs'
$userData = Join-Path $Build 'user_data_ab'
New-Item -ItemType Directory -Force $userData | Out-Null
foreach ($v in $Variants) {
  $before = @(Get-ChildItem $logs -Filter 'simpsons_*.log' -ErrorAction SilentlyContinue | ForEach-Object { $_.Name })
  $arguments = @("--game_data_root=`"$GameData`"", "--user_data_root=`"$userData`"", '--any360_gpu=true',
    '--any360_sys_groups=stateless,files,objects,threads,memory,xam,input,audio') + ($v -split ' ' | Where-Object { $_ -and $_ -notlike '@exe:*' })
  if ($Device) { $arguments += "--any360_gpu_device=$Device" }
  $exe = 'simpsons.exe'
  if ($v -match '^@exe:(\S+)') { $exe = $Matches[1] }
  $p = Start-Process -FilePath (Join-Path $Build $exe) -ArgumentList $arguments -WorkingDirectory $Build -WindowStyle Minimized -PassThru
  if ($Affinity) { $p.ProcessorAffinity = [IntPtr]$Affinity }  # every thread, those already started included
  for ($i = 0; $i -lt $Seconds -and -not $p.HasExited; $i++) { Start-Sleep -Seconds 1 }
  if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force } else { "exited (code $($p.ExitCode))" }
  Start-Sleep -Seconds 2
  $log = Get-ChildItem $logs -Filter 'simpsons_*.log' | Where-Object { $before -notcontains $_.Name } | Sort-Object LastWriteTime | Select-Object -Last 1
  "=== $v ($($log.Name))"
  $l = Get-Content $log.FullName
  "hangs: $(($l | Select-String '\[HANG\]').Count); errors: $(($l | Select-String '\[error\]').Count)"
  $l | Select-String 'Any360: [0-9.]+ FPS; per frame' | Select-Object -Last 2 | ForEach-Object { $x = $_.Line; $i0 = $x.IndexOf('Any360:'); $x.Substring($i0, [Math]::Min(110, $x.Length - $i0)) }
  $l | Select-String 'calls verified' | Select-Object -Last 1 | ForEach-Object { $x = $_.Line; $x.Substring($x.IndexOf('Any360 xdk')) }
  $l | Select-String 'Any360 xdk bench: .*calls, own code' | Select-Object -Last 12 | ForEach-Object { $x = $_.Line; $x.Substring($x.IndexOf('Any360 xdk'), [Math]::Min(220, $x.Length - $x.IndexOf('Any360 xdk'))) }
}
# Only this script's launches (their user data folder), never another session's or the player's.
Get-CimInstance Win32_Process -Filter "name like 'simpsons%.exe'" | Where-Object { $_.CommandLine -like "*user_data_ab*" -and $_.CommandLine -like "*$Build*" } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force; 'killed stray' }
