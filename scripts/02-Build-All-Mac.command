#!/bin/bash
set -euo pipefail

scriptDir="$(cd "$(dirname "$0")" && pwd)"
repoRoot="$(cd "$scriptDir/.." && pwd)"
makeRoot="$repoRoot/build/ProjectFiles/gmake"

if [[ ! -f "$repoRoot/scripts/bootstrapDeps.sh" || ! -f "$repoRoot/scripts/generateProjectFiles.sh" ]]; then
  echo "Missing helper scripts under $repoRoot/scripts."
  echo "Expected:"
  echo "  bootstrapDeps.sh"
  echo "  generateProjectFiles.sh"
  exit 1
fi

echo "[1/3] Syncing dependencies..."
bash "$repoRoot/scripts/bootstrapDeps.sh"

echo "[2/3] Generating GNU Make project files..."
bash "$repoRoot/scripts/generateProjectFiles.sh" gmake

echo "[3/3] Building Debug and Release..."
rm -rf "$repoRoot/build/Intermediate" "$repoRoot/build/Build"
make -C "$makeRoot" config=debug
make -C "$makeRoot" config=release

echo
echo "Build succeeded."
echo "Debug executable:   $repoRoot/build/Build/gmake/Debug/metalCppTest"
echo "Release executable: $repoRoot/build/Build/gmake/Release/metalCppTest"
if [[ -z "${CI:-}" && -t 0 ]]; then
  read -r -n 1 -p "Press any key to close..."
  echo
fi
