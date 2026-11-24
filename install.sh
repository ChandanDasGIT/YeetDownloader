#!/usr/bin/env bash
# Builds YeetDownloader and installs it for the current user, without sudo:
# into ~/.local, or into PREFIX if set (PREFIX=/some/dir ./install.sh).
set -e

if [ "$EUID" -eq 0 ]; then
  echo "Run this without sudo: it installs for your user only."
  exit 1
fi

cd "$(dirname "$0")"
PREFIX="${PREFIX:-$HOME/.local}"
BUILD_DIR="build"

cmake -B "$BUILD_DIR" -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX"
cmake --build "$BUILD_DIR" --parallel
cmake --install "$BUILD_DIR"

# Refresh the menu and icon caches so the launcher shows up immediately
if command -v update-desktop-database &> /dev/null; then
  update-desktop-database "$PREFIX/share/applications" 2>/dev/null || true
fi
if command -v gtk-update-icon-cache &> /dev/null; then
  gtk-update-icon-cache -f -t "$PREFIX/share/icons/hicolor" 2>/dev/null || true
fi

echo "YeetDownloader installed to $PREFIX"
