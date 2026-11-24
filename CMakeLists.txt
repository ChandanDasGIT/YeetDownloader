cmake_minimum_required(VERSION 3.16)

project(YeetDownloader VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(YEET_WARNINGS /W3)
    add_compile_options(/utf-8)
else()
    set(YEET_WARNINGS -Wall -Wextra)
endif()
if(WIN32)
    # Windows' headers would otherwise define min / max as macros
    add_compile_definitions(NOMINMAX)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml Network QuickControls2)
find_package(Threads REQUIRED)

qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

qt_standard_project_setup()

# Backend: pure C++, must never link Qt
add_library(backend STATIC
    backend/downloadPanel/processRunner.h
    backend/downloadPanel/processRunner.cpp
    backend/downloadPanel/sha256.h
    backend/downloadPanel/sha256.cpp
    backend/downloadPanel/ytdlpBinary.h
    backend/downloadPanel/ytdlpBinary.cpp
    backend/downloadPanel/ytdlpCommand.h
    backend/downloadPanel/ytdlpCommand.cpp
    backend/downloadPanel/ytdlpOutputParser.h
    backend/downloadPanel/ytdlpOutputParser.cpp
)
if(WIN32)
    target_sources(backend PRIVATE backend/downloadPanel/processRunner_windows.cpp)
else()
    target_sources(backend PRIVATE backend/downloadPanel/processRunner_posix.cpp)
endif()
set_target_properties(backend PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
target_include_directories(backend PUBLIC ${CMAKE_SOURCE_DIR})
target_compile_options(backend PRIVATE ${YEET_WARNINGS})
target_link_libraries(backend PRIVATE Threads::Threads)

# Clean executable target name that won't collide with the module folder
qt_add_executable(appYeetDownloader
    main.cpp
    wrapper/downloadPanel/downloadPanelWrap.h
    wrapper/downloadPanel/downloadPanelWrap.cpp
)

if(WIN32)
    # The .exe's icon, and no console window behind the app
    target_sources(appYeetDownloader PRIVATE release/windows/YeetDownloader.rc)
    set_target_properties(appYeetDownloader PROPERTIES WIN32_EXECUTABLE TRUE OUTPUT_NAME YeetDownloader)
endif()

qt_add_qml_module(appYeetDownloader
    URI YeetDownloader
    VERSION 1.0
    QML_FILES
        frontend/Main.qml
        frontend/downloadPanel/DownloadPanel.qml
        frontend/downloadPanel/StatusCard.qml
)

target_link_libraries(appYeetDownloader PRIVATE
    backend
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::Network
    Qt6::QuickControls2
)

# -------------------------------------------------------------
# Installation Rules (Linux; Windows uses release/windows/YeetDownloader.iss)
# -------------------------------------------------------------
include(GNUInstallDirs)

# 1. Install the executable binary
install(TARGETS appYeetDownloader
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(UNIX AND NOT APPLE)
    # 2. Install application icon
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/app_icon.png"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps"
        RENAME "yeetdownloader.png"
    )

    # 3. Generate and install a .desktop launcher for Linux menus
    # (full path: ~/.local/bin is not always on the menu's PATH)
    set(DESKTOP_FILE_CONTENT
"[Desktop Entry]
Type=Application
Name=YeetDownloader
Comment=Video and Audio Downloader
Exec=\"${CMAKE_INSTALL_FULL_BINDIR}/appYeetDownloader\"
Icon=yeetdownloader
Terminal=false
Categories=Network;AudioVideo;Utility;
")

    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/yeetdownloader.desktop" "${DESKTOP_FILE_CONTENT}")

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/yeetdownloader.desktop"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
    )
endif()
