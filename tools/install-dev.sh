#!/usr/bin/env bash
# Development install on the workstation, without a package: the filter
# goes to ~/config/non-packaged/add-ons/input_server/filters (input_server
# picks it up on its own) and the application runs from the build folder.
# Usage: tools/install-dev.sh [--run]
set -euo pipefail
AIRSHOT_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
REMOTE_DIR=${AIRSHOT_REMOTE_DIR:-/boot/home/airshot}
bash "$AIRSHOT_ROOT/tools/ws.sh" "
set -e
cd $REMOTE_DIR
hey application/x-vnd.airOS-airShot quit >/dev/null 2>&1 || true
sleep 1
FILTERS=/boot/home/config/non-packaged/add-ons/input_server/filters
mkdir -p \$FILTERS
# Replace the filter atomically; input_server reloads it on the change.
rm -f \$FILTERS/airShot
cp build-haiku/airShot_filter \$FILTERS/airShot
mkdir -p /boot/home/config/non-packaged/apps
cp build-haiku/airShot /boot/home/config/non-packaged/apps/airShot
echo installed
"
if [[ ${1:-} == --run ]]; then
	bash "$AIRSHOT_ROOT/tools/ws.sh" "nohup /boot/home/config/non-packaged/apps/airShot >/boot/home/airshot/app.log 2>&1 &
sleep 2; ps | grep -i airshot | grep -v grep || true"
fi
