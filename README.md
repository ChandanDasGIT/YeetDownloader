# YeetDownloader

A lightweight Qt/QML frontend for downloading audio and video via `yt-dlp`.

## Prerequisites

Make sure the following dependencies are installed before building:

* **Qt 6** (with Quick, QML, Network, and QuickControls2 modules)
* **CMake** (version 3.16 or newer)
* **C++17 compiler** (GCC, Clang, etc.)
* **yt-dlp** and **ffmpeg** (for downloading and processing media)

### Debian / Ubuntu:
sudo apt update
sudo apt install build-essential cmake qt6-base-dev qt6-declarative-dev qt6-declarative-dev-tools libqt6network6 yt-dlp ffmpeg

### Fedora:
sudo dnf install gcc-c++ cmake qt6-qtbase-devel qt6-qtdeclarative-devel yt-dlp ffmpeg

### Arch Linux:
sudo pacman -S base-devel cmake qt6-base qt6-declarative yt-dlp ffmpeg

### INSTALLATION: From the .tar.xz Archive

  # 1. Extract the archive
  tar -xf YeetDownloader-source.tar.xz
  
  # 2. Enter the folder
  cd YtDlpApp
  
  # 3. Run the installer
  chmod +x install.sh
  sudo ./install.sh

### INSTALLATION: From Git
  git clone https://github.com/ChandanDasGIT/YeetDownloader.git
  cd YeetDownloader
  chmod +x install.sh
  sudo ./install.sh

### Open App
  bash: appYeetDownloader
  search in app menu: YeetDownloader

### Uninstallation:
  chmod +x uninstall.sh
  sudo ./uninstall.sh
  
