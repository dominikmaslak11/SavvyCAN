#!/bin/bash
# SavvyCAN — create portable 7z archive for distribution
# Usage: ./make_portable.sh [debug|release]

BUILD_DIR="${1:-build_dbg}"
ARCHIVE_NAME="SavvyCAN-portable.7z"

if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: build directory '$BUILD_DIR' not found"
    echo "Available:"
    ls -d build* 2>/dev/null || echo "  (none)"
    exit 1
fi

if [ ! -f "$BUILD_DIR/SavvyCAN.exe" ]; then
    echo "Error: SavvyCAN.exe not found in $BUILD_DIR"
    echo "Build first: cmake --build $BUILD_DIR"
    exit 1
fi

cd "$(dirname "$0")/$BUILD_DIR"

echo "Creating $ARCHIVE_NAME from $BUILD_DIR ..."
/usr/bin/7z a -mx9 "../${ARCHIVE_NAME}" \
  SavvyCAN.exe *.dll \
  platforms canbus imageformats styles tls networkinformation generic qmltooling qml \
  iconengines 2>&1

echo ""
echo "Archive size:"
ls -lh "../${ARCHIVE_NAME}"
echo "Done."
