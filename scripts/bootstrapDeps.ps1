Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptRoot "..")
$thirdPartyRoot = Join-Path $repoRoot "thirdParty"

function SyncGitRepo {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][string]$Url,
        [Parameter(Mandatory = $true)][string]$Commit
    )

    $targetPath = Join-Path $thirdPartyRoot $Name
    if (!(Test-Path $targetPath)) {
        Write-Host "Cloning $Name..."
        git clone $Url $targetPath
    }

    Write-Host "Pinning $Name to $Commit..."
    git -C $targetPath fetch --depth 1 origin $Commit
    git -C $targetPath checkout --detach $Commit
}

New-Item -ItemType Directory -Force -Path $thirdPartyRoot | Out-Null

SyncGitRepo -Name "glfw" -Url "https://github.com/glfw/glfw.git" -Commit "7b6aead9fb88b3623e3b3725ebb42670cbe4c579"
SyncGitRepo -Name "imgui" -Url "https://github.com/ocornut/imgui.git" -Commit "3912b3d9a9c1b3f17431aebafd86d2f40ee6e59c"
SyncGitRepo -Name "metal-cpp" -Url "https://github.com/bkaradzic/metal-cpp.git" -Commit "5caea74c5f77492add32b7cad109d796e342ab49"
SyncGitRepo -Name "EASTL" -Url "https://github.com/electronicarts/EASTL.git" -Commit "9d2e8a090bceae2bb658bc45c3d4ee2d796cdf48"
SyncGitRepo -Name "EABase" -Url "https://github.com/electronicarts/EABase.git" -Commit "0699a15efdfd20b6cecf02153bfa5663decb653c"
SyncGitRepo -Name "uWebSockets" -Url "https://github.com/uNetworking/uWebSockets.git" -Commit "v20.72.0"
SyncGitRepo -Name "uSockets" -Url "https://github.com/uNetworking/uSockets.git" -Commit "v0.8.8"

Write-Host "Dependency bootstrap complete."
