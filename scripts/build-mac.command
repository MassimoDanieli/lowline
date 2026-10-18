#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
if ! xcode-select -p >/dev/null 2>&1; then
 echo 'Install Apple Command Line Tools first: xcode-select --install'
 exit 1
fi
if ! command -v cmake >/dev/null 2>&1; then
 echo 'CMake is required. Install from https://cmake.org/download/ or run: brew install cmake'
 exit 1
fi
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="$(uname -m)"
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
open build/Lowline_artefacts/Release
