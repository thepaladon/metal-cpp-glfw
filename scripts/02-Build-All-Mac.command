#!/bin/bash
set -euo pipefail

scriptDir="$(cd "$(dirname "$0")" && pwd)"
repoRoot="$(cd "$scriptDir/.." && pwd)"
makeRoot="$repoRoot/build/ProjectFiles/gmake2"

if [[ ! -f "$repoRoot/scripts/bootstrapDeps.sh" || ! -f "$repoRoot/scripts/generateProjectFiles.sh" ]]; then
  echo "Missing helper scripts under $repoRoot/scripts."
  echo "Expected:"
  echo "  bootstrapDeps.sh"
  echo "  generateProjectFiles.sh"
  exit 1
fi

echo "[1/3] Syncing dependencies..."
"$repoRoot/scripts/bootstrapDeps.sh"

echo "[2/3] Generating GNU Make project files..."
"$repoRoot/scripts/generateProjectFiles.sh" gmake2

echo "[3/3] Building Debug and Release..."
make -C "$makeRoot" config=debug
make -C "$makeRoot" config=release

echo
echo "Build succeeded."
echo "Debug executable:   $repoRoot/build/Build/Debug/metalCppTest"
echo "Release executable: $repoRoot/build/Build/Release/metalCppTest"
if [[ -z "${CI:-}" ]]; then
  read -r -n 1 -p "Press any key to close..."
  echo
fi
