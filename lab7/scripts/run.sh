#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
PROJECT_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd)
BIN="$PROJECT_DIR/build/lab_07"

if [[ ! -x "$BIN" ]]; then
  echo "Executable not found. Run ./build.sh first." >&2
  exit 1
fi

CAMERA_INDEX="${1:-0}"
cd "$PROJECT_DIR"
"$BIN" "$CAMERA_INDEX"
