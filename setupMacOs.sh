#!/usr/bin/env bash
set -euo pipefail

scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repoRoot="$scriptDir"

requireCommand() {
  local commandName="$1"
  if ! command -v "$commandName" >/dev/null 2>&1; then
    return 1
  fi
}

if ! xcode-select -p >/dev/null 2>&1; then
  echo "Xcode Command Line Tools are missing. Run: xcode-select --install"
  exit 1
fi

if ! requireCommand brew; then
  echo "Homebrew is required. Install it from https://brew.sh and re-run this script."
  exit 1
fi

if ! requireCommand cmake; then
  echo "Installing cmake..."
  brew install cmake
fi

if ! requireCommand ninja; then
  echo "Installing ninja..."
  brew install ninja
fi

cd "$repoRoot"

echo "Configuring Ninja workspace..."
cmake --preset ninja

echo "Building Debug target..."
cmake --build --preset ninja-debug

echo "Done. Executable: $repoRoot/build/Build/Debug/metal_cpp_test"
