#!/usr/bin/env bash
# Sync the sources to the workstation and build there, printing compiler
# errors only. Usage: tools/sync-build.sh [make-target] [max-lines]
set -uo pipefail
AIRSHOT_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
cd "$AIRSHOT_ROOT"
TARGET=${1:-all}
REMOTE_DIR=${AIRSHOT_REMOTE_DIR:-/boot/home/airshot}
tar -czf - --exclude=__pycache__ Makefile README.md LICENSE docs src filter tests tools resources 2>/dev/null |
    bash tools/ws.sh "mkdir -p $REMOTE_DIR && tar xzf - --warning=no-timestamp -C $REMOTE_DIR 2>/dev/null; cd $REMOTE_DIR && make -j8 $TARGET 2>&1 | grep -E -B1 -A4 'error|Error|undefined|warning' | head -${2:-80}; echo \"exit: \${PIPESTATUS[0]}\""
