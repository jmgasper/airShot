#!/usr/bin/env bash
# Run a command on the Haiku workstation (192.168.1.244) where airShot is
# built and tested. Usage: tools/ws.sh 'command'   (stdin is forwarded)
set -euo pipefail
AIRSHOT_WS_HOST=${AIRSHOT_WS_HOST:-192.168.1.244}
AIRSHOT_WS_USER=${AIRSHOT_WS_USER:-user}
AIRSHOT_WS_KEY=${AIRSHOT_WS_KEY:-/mnt/HaikuWork/x399/ssh/workstation_ed25519}
exec ssh -i "$AIRSHOT_WS_KEY" -o IdentitiesOnly=yes -o BatchMode=yes \
    -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -o LogLevel=ERROR \
    -o ConnectTimeout=15 -o ServerAliveInterval=30 -o ServerAliveCountMax=8 \
    "$AIRSHOT_WS_USER@$AIRSHOT_WS_HOST" "$@"
