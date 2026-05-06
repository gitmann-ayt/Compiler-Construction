param(
  [switch]$Clean,
  [ValidateSet('Debug','Release')] [string]$Config = 'Debug'
)

$ErrorActionPreference = 'Stop'

if ($Clean -and (Test-Path build)) {
  Remove-Item -Recurse -Force build
}

cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=$Config
cmake --build build

Write-Host "Built binaries in .\\build\\" -ForegroundColor Green
