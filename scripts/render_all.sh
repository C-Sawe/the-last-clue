#!/usr/bin/env bash
# Builds the project and runs every demo, writing all images to output/.
# Usage (from the repository root):  bash scripts/render_all.sh
set -euo pipefail
cmake -B build -S . >/dev/null
cmake --build build -j
for exe in build/[0-9]*_* build/Debug/[0-9]*_*.exe build/Release/[0-9]*_*.exe; do
  [[ -f "$exe" && -x "$exe" ]] && "$exe"
done
echo "All images are in output/"
