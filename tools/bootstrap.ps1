# Checks that everything BeastVR needs is installed. Changes nothing.
#   .\tools\bootstrap.ps1
$script:ok = $true

function Check([string]$name, [scriptblock]$test, [string]$fix) {
  if (& $test) {
    Write-Host "[ok]    $name" -ForegroundColor Green
  } else {
    Write-Host "[miss]  $name   ->  $fix" -ForegroundColor Red
    $script:ok = $false
  }
}

Check 'Git'        { Get-Command git   -ErrorAction SilentlyContinue } 'winget install Git.Git'
Check 'GitHub CLI' { Get-Command gh    -ErrorAction SilentlyContinue } 'winget install GitHub.cli'
Check 'CMake'      { Get-Command cmake -ErrorAction SilentlyContinue } 'winget install Kitware.CMake'
Check 'Ninja'      { Get-Command ninja -ErrorAction SilentlyContinue } 'winget install Ninja-build.Ninja'

Check 'Visual Studio C++ tools' {
  $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
  (Test-Path $vswhere) -and (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath)
} 'Install VS 2022 with "Desktop development with C++" (docs\SETUP.md step 1)'

$runtime = (Get-ItemProperty 'HKLM:\SOFTWARE\Khronos\OpenXR\1' -ErrorAction SilentlyContinue).ActiveRuntime
if ($runtime) {
  Write-Host "[ok]    OpenXR runtime: $runtime" -ForegroundColor Green
} else {
  Write-Host '[warn]  No active OpenXR runtime. Set Virtual Desktop, Quest Link or SteamVR as active (docs\SETUP.md step 2).' -ForegroundColor Yellow
}

if ($script:ok) {
  Write-Host "`nAll set. Next: .\tools\build.ps1" -ForegroundColor Green
} else {
  Write-Host "`nInstall the missing tools, reopen PowerShell, and run this again." -ForegroundColor Yellow
  exit 1
}
