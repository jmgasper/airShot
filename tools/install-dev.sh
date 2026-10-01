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
# An editor asking about unsaved work would block the quit.
for team in \$(ps | grep '^/boot/home/config/non-packaged/apps/airShot' | awk '{print \$2}'); do kill \$team 2>/dev/null || true; done
sleep 1
FILTERS=/boot/home/config/non-packaged/add-ons/input_server/filters
mkdir -p \$FILTERS
# input_server's add-on monitor loads a *created* entry but ignores a
# renamed one, so remove the old filter and copy the new one into place.
rm -f \$FILTERS/airShot
sleep 1
cp build-haiku/airShot_filter \$FILTERS/airShot
sleep 2
mkdir -p /boot/home/config/non-packaged/apps
# Deskbar keeps the old executable mapped for the tray icon: never overwrite
# it in place, swap in a new file instead.
cp build-haiku/airShot /boot/home/config/non-packaged/apps/airShot.new
mv -f /boot/home/config/non-packaged/apps/airShot.new /boot/home/config/non-packaged/apps/airShot
echo installed
"
if [[ ${1:-} == --run ]]; then
	bash "$AIRSHOT_ROOT/tools/ws.sh" "AIRSHOT_TRACE=1 nohup /boot/home/config/non-packaged/apps/airShot >/boot/home/airshot/app.log 2>&1 &
sleep 2; ps | grep -i airshot | grep -v grep || true"
fi
