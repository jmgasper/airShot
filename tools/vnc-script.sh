#!/usr/bin/env bash
# Runs a list of vnc.py commands, one per line on stdin, against the
# workstation. Lines starting with "sleep" wait; "shot NAME X,Y,W,H" saves a
# crop to $SHOTS/NAME.png. Example:
#   printf 'drag 700 450 1000 650\ntype R\nshot after 600,380,900,400\n' | tools/vnc-script.sh
set -u
AIRSHOT_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
VNC="python3 $AIRSHOT_ROOT/tools/vnc.py"
SHOTS=${SHOTS:-$AIRSHOT_ROOT/artifacts/shots}
mkdir -p "$SHOTS"
while IFS= read -r line; do
	[[ -z $line ]] && continue
	set -- $line
	case $1 in
		sleep) sleep "$2" ;;
		shot) $VNC shot "$SHOTS/$2.png" --rect "$3" ${4:-} >/dev/null && echo "shot $SHOTS/$2.png" ;;
		full) $VNC shot "$SHOTS/$2.png" --scale "${3:-0.5}" >/dev/null && echo "shot $SHOTS/$2.png" ;;
		type) shift; $VNC type "$*" ;;
		*) $VNC "$@" ;;
	esac
	sleep 0.25
done
