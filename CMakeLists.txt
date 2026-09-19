cmake_minimum_required(VERSION 3.16)

project(YeetDownloader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml Network QuickControls2)

qt_policy(SET QTP0001 NEW)

qt_standard_project_setup()

# Clean executable target name that won't collide with the module folder
qt_add_executable(appYeetDownloader
    main.cpp
    downloader.h
    downloader.cpp
)

qt_add_qml_module(appYeetDownloader
    URI YeetDownloader
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(appYeetDownloader PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::Network
    Qt6::QuickControls2
)

# -------------------------------------------------------------
# Installation Rules
# -------------------------------------------------------------
include(GNUInstallDirs)

# 1. Install the executable binary
install(TARGETS appYeetDownloader
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 2. Install application icon
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/app_icon.png"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps"
    RENAME "yeetdownloader.png"
)

# 3. Generate and install a .desktop launcher for Linux menus
set(DESKTOP_FILE_CONTENT
"[Desktop Entry]
Type=Application
Name=YeetDownloader
Comment=Video and Audio Downloader
Exec=appYeetDownloader
Icon=yeetdownloader
Terminal=false
Categories=Network;AudioVideo;Utility;
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/yeetdownloader.desktop" "${DESKTOP_FILE_CONTENT}")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/yeetdownloader.desktop"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
)