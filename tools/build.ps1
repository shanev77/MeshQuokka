param([string]$Environment = 'MeshCore_Enhanced_TDeck_Plus')
$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$workspaceRoot = (Resolve-Path (Join-Path $projectRoot '..')).Path
$projectDirectory = Split-Path $projectRoot -Leaf
$python = Join-Path $workspaceRoot '.venv\Scripts\python.exe'
& $python (Join-Path $PSScriptRoot 'sync_application.py')
if ($LASTEXITCODE -ne 0) { throw 'Application sync failed' }
# GCC 8 on Windows can exceed CreateProcess's command-line limit with long
# OneDrive paths. A temporary drive alias shortens all framework include paths.
$drive = @('M:', 'N:', 'O:', 'P:', 'Q:', 'R:') | Where-Object { -not (Test-Path ($_ + '\')) } | Select-Object -First 1
if (-not $drive) { throw 'No temporary build drive available' }
$previousCore = $env:PLATFORMIO_CORE_DIR
& subst.exe $drive $workspaceRoot
if ($LASTEXITCODE -ne 0) { throw 'Cannot create temporary workspace drive' }
try {
    $env:PLATFORMIO_CORE_DIR = "$drive\.platformio"
    & $python -m platformio run -d "$drive\$projectDirectory\firmware" -e $Environment -j 4
    if ($LASTEXITCODE -ne 0) { throw 'Firmware compilation failed; see compiler output' }
} finally {
    $env:PLATFORMIO_CORE_DIR = $previousCore
    & subst.exe $drive /D
}
