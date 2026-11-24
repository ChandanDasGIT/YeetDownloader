# YeetDownloader

A lightweight Qt/QML frontend for downloading audio and video via `yt-dlp`, for Linux and Windows.

## Features

* Download videos (MP4, up to the quality you pick) or audio (MP3)
* Whole playlists of any length, with "Video X of Y" progress
* Playlist files are numbered in playlist order (`001 - Title [id].mp4`)
* Videos already in the download folder are skipped, so a stopped playlist can simply be started again
* Cancel button, and an optional pause between videos
* Clear error messages (yt-dlp's own) and a summary at the end
* Remembers your download folder
* Sets up and updates its own copy of `yt-dlp`, checked against yt-dlp's published SHA-256 checksums

## Windows

Download `YeetDownloader-<version>-setup-x64.exe` from the [Releases](https://github.com/ChandanDasGIT/YeetDownloader/releases) page and run it.

* Installs for your user only, no administrator rights needed
* If `ffmpeg` is missing, the installer offers to install it with `winget`. If that isn't possible, open a terminal and run:

      winget install Gyan.FFmpeg

## Linux

### Prerequisites

Make sure the following dependencies are installed before building:

* **Qt 6.5 or newer** (with Quick, QML, Network, and QuickControls2 modules)
* **CMake** (version 3.16 or newer)
* **C++17 compiler** (GCC, Clang, etc.)
* **Python 3** (to run `yt-dlp`, which the app downloads by itself)
* **ffmpeg** (for making MP3 and MP4 files)

### Debian / Ubuntu:
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-declarative-dev qml6-module-qtquick-controls qml6-module-qtquick-layouts qml6-module-qtquick-dialogs qml6-module-qtcore python3 ffmpeg

### Fedora:
sudo dnf install gcc-c++ cmake qt6-qtbase-devel qt6-qtdeclarative-devel python3 ffmpeg

### Arch Linux:
sudo pacman -S base-devel cmake qt6-base qt6-declarative python ffmpeg

### INSTALLATION: From the .tar.xz Archive

The installer needs no `sudo`: it installs for your user into `~/.local`.

  # 1. Extract the archive
  tar -xf YeetDownloader-source.tar.xz

  # 2. Enter the folder
  cd YeetDownloader

  # 3. Run the installer
  chmod +x install.sh
  ./install.sh

### INSTALLATION: From Git
  git clone https://github.com/ChandanDasGIT/YeetDownloader.git

  cd YeetDownloader

  chmod +x install.sh

  ./install.sh

To install somewhere else, set `PREFIX`, e.g. `PREFIX=/opt/yeet ./install.sh`.

### Open App
  bash: ~/.local/bin/appYeetDownloader (or appYeetDownloader, if ~/.local/bin is on your PATH)
  search in app menu: YeetDownloader

### Uninstallation:
  chmod +x uninstall.sh
  ./uninstall.sh

### Upgrading from 1.0

Version 1.0 was installed system-wide with `sudo`. Remove it once before installing 2.0:

  sudo rm /usr/local/bin/appYeetDownloader /usr/local/share/icons/hicolor/256x256/apps/yeetdownloader.png /usr/local/share/applications/yeetdownloader.desktop
