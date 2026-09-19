#!/usr/bin/env bash
set -e

if [ "$EUID" -ne 0 ]; then
  echo "Please run as root (e.g., sudo ./uninstall.sh)"
  exit 1
fi

if [ -f "build/install_manifest.txt" ]; then
  xargs rm -vf < build/install_manifest.txt
  echo "YeetDownloader uninstalled successfully."
else
  echo "Error: build/install_manifest.txt not found."
  exit 1
fi