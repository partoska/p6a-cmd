#!/usr/bin/env bash
# Command Line Interface for Partoska.com media sharing service.
# Copyright (C) 2026 Fabrika Charvat s.r.o. All rights reserved.
# Developed by Partoska Laboratory team, <https://lab.partoska.com>
#
# MIT License
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# You can contact the author(s) via email at ask <at> partoska.com.
#

# Downloads the pinned Bikini release archive of a platform, checks its
# SHA-256 and unpacks it to DEST, so that DEST/include/bikini.h and
# DEST/lib/libbikini.a exist. Point CMAKE_PREFIX_PATH (CMake) or
# BIKINI_PREFIX (make) at DEST.
#
# Usage: fetch-bikini.sh PLATFORM DEST
#
#   PLATFORM  linux_amd64, linux_arm64, linux_armhf, darwin_universal,
#             windows_amd64 or windows_arm64

set -euo pipefail

# Bump the version together with every checksum, taken from the SHA256SUMS
# of the release.
VERSION="2.0.1"

if [ $# -ne 2 ]; then
  echo "usage: $0 PLATFORM DEST" >&2
  exit 2
fi
PLATFORM="$1"
DEST="$2"

case "$PLATFORM" in
  linux_amd64) EXT=tar.gz SUM=762106adca5fe94c2bcde6aebac03903a600151443e94fe2146a3190beae857f ;;
  linux_arm64) EXT=tar.gz SUM=573f8f7ad6e1dc4186c4347ee38151a284dd2b066112b03458715aebc0c1b17b ;;
  linux_armhf) EXT=tar.gz SUM=5032ff6f21a95efbc0e4df33c49a8ff0d8f34421d2a409497194a7bd5263bbad ;;
  darwin_universal) EXT=tar.gz SUM=74fd960c9554c7fa8bd5c425b0262a1879c20bda831d4e1b52cd20c3ea2a8527 ;;
  windows_amd64) EXT=zip SUM=728f845b9151c7b93c2a119edc3205d87f4dbd37b3dcad41b8fe24b3c8039759 ;;
  windows_arm64) EXT=zip SUM=c1c390ee1059011a685e5c9593f4fc820a9e165d8377927dc194ae7000c5cdcb ;;
  *)
    echo "$0: unknown platform '$PLATFORM'" >&2
    exit 2
    ;;
esac

NAME="bikini_${VERSION}_${PLATFORM}.${EXT}"
URL="https://github.com/partoska/bikini/releases/download/v${VERSION}/${NAME}"

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

curl -fsSL --retry 3 -o "$TMP/$NAME" "$URL"

if command -v sha256sum >/dev/null 2>&1; then
  ACTUAL=$(sha256sum "$TMP/$NAME" | cut -d' ' -f1)
else
  ACTUAL=$(shasum -a 256 "$TMP/$NAME" | cut -d' ' -f1)
fi
if [ "$ACTUAL" != "$SUM" ]; then
  echo "$0: checksum mismatch for $NAME: expected $SUM, got $ACTUAL" >&2
  exit 1
fi

mkdir -p "$TMP/unpacked"
if [ "$EXT" = zip ]; then
  if command -v unzip >/dev/null 2>&1; then
    unzip -q "$TMP/$NAME" -d "$TMP/unpacked"
  else
    7z x -bd -y -o"$TMP/unpacked" "$TMP/$NAME" >/dev/null
  fi
else
  tar -xzf "$TMP/$NAME" -C "$TMP/unpacked"
fi

# The macOS archive nests the prefix under usr/local, the others do not.
HEADER=$(find "$TMP/unpacked" -path '*/include/bikini.h' | head -n 1)
if [ -z "$HEADER" ]; then
  echo "$0: $NAME holds no include/bikini.h" >&2
  exit 1
fi

rm -rf "$DEST"
mkdir -p "$(dirname "$DEST")"
mv "$(dirname "$(dirname "$HEADER")")" "$DEST"

echo "Bikini $VERSION ($PLATFORM) in $DEST"
