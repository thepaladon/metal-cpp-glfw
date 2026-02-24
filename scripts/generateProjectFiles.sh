#!/usr/bin/env bash
set -euo pipefail

action="${1:-gmake}"

if [[ "$action" == "gmake2" ]]; then
  action="gmake"
fi
if [[ "$action" == "web" ]]; then
  action="gmake"
  export AA_WEB_BUILD=1
fi
case "$action" in
  gmake|xcode4) ;;
  *)
    echo "Unsupported action: $action"
    echo "Usage: ./scripts/generateProjectFiles.sh [gmake|xcode4|web]"
    exit 1
    ;;
esac

if ! command -v premake5 >/dev/null 2>&1; then
  echo "premake5 not found in PATH. Install Premake first: https://premake.github.io/download"
  exit 1
fi

scriptRoot="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repoRoot="$(cd "$scriptRoot/.." && pwd)"
mkdir -p "$repoRoot/build/ProjectFiles"

pushd "$repoRoot" >/dev/null
premake5 "$action"
popd >/dev/null
