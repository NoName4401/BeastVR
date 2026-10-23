# Builds BeastVR into build\bin. Run from anywhere:  .\tools\build.ps1
#   -Config Debug | RelWithDebInfo (default) | Release
param(
  [ValidateSet('Debug', 'RelWithDebInfo', 'Release')]
  [string]$Config = 'RelWithDebInfo'
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot

# Enter the Visual Studio x64 developer environment if we aren't in it already.
if (-not $env:VCINSTALLDIR) {
  $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
  if (-not (Test-Path $vswhere)) { throw 'Visual Studio not found. Run tools\bootstrap.ps1 and see docs\SETUP.md.' }
  $vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
  if (-not $vs) { throw 'Visual Studio has no C++ tools. Add "Desktop development with C++" (docs\SETUP.md step 1).' }
  Import-Module (Join-Path $vs 'Common7\Tools\Microsoft.VisualStudio.DevShell.dll')
  Enter-VsDevShell -VsInstallPath $vs -SkipAutomaticLocation -DevCmdArguments '-arch=x64 -host_arch=x64' | Out-Null
}

$build = Join-Path $root 'build'
cmake -S $root -B $build -G Ninja "-DCMAKE_BUILD_TYPE=$Config"
if ($LASTEXITCODE) { throw 'CMake configure failed.' }

cmake --build $build
if ($LASTEXITCODE) { throw 'Build failed.' }

Write-Host "`nBuilt ($Config) into $build\bin" -ForegroundColor Green
