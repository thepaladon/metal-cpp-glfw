#!/usr/bin/env bash
set -euo pipefail

scriptRoot="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repoRoot="$(cd "$scriptRoot/.." && pwd)"
thirdPartyRoot="$repoRoot/thirdParty"

syncGitRepo() {
    local name="$1"
    local url="$2"
    local commit="$3"
    local targetPath="$thirdPartyRoot/$name"

    if [[ ! -d "$targetPath" ]]; then
        echo "Cloning $name..."
        git clone "$url" "$targetPath"
    fi

    echo "Pinning $name to $commit..."
    git -C "$targetPath" fetch --depth 1 origin "$commit"
    git -C "$targetPath" checkout --detach "$commit"
}

mkdir -p "$thirdPartyRoot"

syncGitRepo "glfw" "https://github.com/glfw/glfw.git" "7b6aead9fb88b3623e3b3725ebb42670cbe4c579"
syncGitRepo "imgui" "https://github.com/ocornut/imgui.git" "3912b3d9a9c1b3f17431aebafd86d2f40ee6e59c"
syncGitRepo "metal-cpp" "https://github.com/bkaradzic/metal-cpp.git" "5caea74c5f77492add32b7cad109d796e342ab49"
syncGitRepo "EASTL" "https://github.com/electronicarts/EASTL.git" "9d2e8a090bceae2bb658bc45c3d4ee2d796cdf48"
syncGitRepo "EABase" "https://github.com/electronicarts/EABase.git" "0699a15efdfd20b6cecf02153bfa5663decb653c"
syncGitRepo "uWebSockets" "https://github.com/uNetworking/uWebSockets.git" "v20.72.0"
syncGitRepo "uSockets" "https://github.com/uNetworking/uSockets.git" "v0.8.8"

echo "Dependency bootstrap complete."
