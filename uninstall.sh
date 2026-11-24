#!/usr/bin/env bash
# Removes what install.sh installed (listed in build/install_manifest.txt).
set -e

cd "$(dirname "$0")"

if [ ! -f "build/install_manifest.txt" ]; then
  echo "Error: build/install_manifest.txt not found."
  exit 1
fi

# (|| [ -n ...]: the manifest's last line has no newline)
while IFS= read -r file || [ -n "$file" ]; do
  rm -vf -- "$file"
done < build/install_manifest.txt

echo "YeetDownloader uninstalled successfully."
