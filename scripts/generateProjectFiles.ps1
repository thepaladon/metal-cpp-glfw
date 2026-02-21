param(
    [ValidateSet("vs2022", "gmake", "gmake2", "xcode4")]
    [string]$Action = "vs2022"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptRoot "..")
$projectFilesDir = Join-Path $repoRoot "build/ProjectFiles"

if (!(Get-Command premake5 -ErrorAction SilentlyContinue)) {
    throw "premake5 not found in PATH. Install Premake first: https://premake.github.io/download"
}

New-Item -ItemType Directory -Force -Path $projectFilesDir | Out-Null

Push-Location $repoRoot
try {
    if ($Action -eq "gmake2") {
        $Action = "gmake"
    }
    premake5 $Action
}
finally {
    Pop-Location
}
