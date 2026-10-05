#!/usr/bin/env bash
# Builds the Haiku package (.hpkg) on Haiku. Run from the repository root.
set -euo pipefail
AIRSHOT_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
cd "$AIRSHOT_ROOT"
NAME=$(awk '$1 == "name" { print $2; exit }' resources/airShot.PackageInfo)
VERSION=$(awk '$1 == "version" { print $2; exit }' resources/airShot.PackageInfo)
ARCH=$(awk '$1 == "architecture" { print $2; exit }' resources/airShot.PackageInfo)
FILE="$AIRSHOT_ROOT/artifacts/$NAME-$VERSION-$ARCH.hpkg"
make -j8
STAGE=$(mktemp -d /tmp/airshot-package-XXXXXX)
trap 'rm -rf -- "$STAGE"' EXIT
DOCS="$STAGE/documentation/packages/airshot"
mkdir -p "$STAGE/apps" "$STAGE/add-ons/input_server/filters" "$DOCS" \
	"$STAGE/data/deskbar/menu/Applications" "$STAGE/data/licenses" "$AIRSHOT_ROOT/artifacts"
cp build-haiku/airShot "$STAGE/apps/airShot"
strip --strip-debug "$STAGE/apps/airShot"
# GNU strip removes the appended Haiku resources; restore them.
xres -o "$STAGE/apps/airShot" build-haiku/airShot.rsrc
cp build-haiku/airShot_filter "$STAGE/add-ons/input_server/filters/airShot"
strip --strip-debug "$STAGE/add-ons/input_server/filters/airShot"
cp resources/airShot.PackageInfo "$STAGE/.PackageInfo"
cp README.md LICENSE "$DOCS/"
cp -R docs "$DOCS/"
cp -R resources/icons "$DOCS/"
cp resources/icons/fontawesome/CC-BY-4.0.txt "$STAGE/data/licenses/CC BY 4.0"
ln -s ../../../../apps/airShot "$STAGE/data/deskbar/menu/Applications/airShot"
( cd "$STAGE" && mimeset --all -f --mimedb data/mime_db --mimedb /boot/system/data/mime_db apps/airShot )
package create -C "$STAGE" "$FILE"
printf '%s\n' "$FILE"
