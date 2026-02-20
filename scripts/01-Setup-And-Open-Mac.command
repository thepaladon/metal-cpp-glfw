#!/bin/bash
set -euo pipefail

scriptDir="$(cd "$(dirname "$0")" && pwd)"
repoRoot="$(cd "$scriptDir/.." && pwd)"

if [[ ! -f "$repoRoot/scripts/bootstrapDeps.sh" || ! -f "$repoRoot/scripts/generateProjectFiles.sh" ]]; then
  echo "Missing helper scripts under $repoRoot/scripts."
  echo "Expected:"
  echo "  bootstrapDeps.sh"
  echo "  generateProjectFiles.sh"
  exit 1
fi

echo "[1/2] Syncing dependencies..."
bash "$repoRoot/scripts/bootstrapDeps.sh"

echo "[2/2] Generating Xcode project files..."
bash "$repoRoot/scripts/generateProjectFiles.sh" xcode4

workspacePath="$repoRoot/build/ProjectFiles/xcode4/metalCppGlfw.xcworkspace"
projectPath="$repoRoot/build/ProjectFiles/xcode4/metalCppGlfw.xcodeproj"

if [[ -d "$workspacePath" ]]; then
  echo "Opening Xcode workspace..."
  open "$workspacePath"
elif [[ -d "$projectPath" ]]; then
  echo "Opening Xcode project..."
  open "$projectPath"
else
  echo "Generated files not found under build/ProjectFiles/xcode4."
  exit 1
fi

echo
echo "Done."
if [[ -z "${CI:-}" ]]; then
  read -r -n 1 -p "Press any key to close..."
  echo
fi
