import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore // Required for persistent Settings

ApplicationWindow {
    id: root
    visible: true
    width: 540
    height: 440
    minimumWidth: 500
    minimumHeight: 420
    title: "YeetDownloader"

    // Persistent storage for user preferences
    Settings {
        id: appSettings
        category: "Appearance"
        property int themeChoice: 0 // 0 = System, 1 = Dark, 2 = Light
    }

    readonly property bool systemIsDark: Application.styleHints.colorScheme === Qt.Dark
    readonly property bool isDarkTheme: appSettings.themeChoice === 1 || (appSettings.themeChoice === 0 && systemIsDark)

    Material.theme: isDarkTheme ? Material.Dark : Material.Light
    Material.accent: Material.Teal
    Material.elevation: 0

    FolderDialog {
        id: folderDialog
        title: "Select Download Location"
        currentFolder: "file://" + downloader.downloadPath
        onAccepted: downloader.downloadPath = selectedFolder
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 22
        spacing: 14

        // Header + Theme Switcher
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "YeetDownloader"
                font.pixelSize: 18
                font.bold: true
                Layout.fillWidth: true
            }

            ComboBox {
                id: themeSelector
                implicitWidth: 120
                flat: true
                model: ["System", "Dark", "Light"]
                // Bind the current selection to saved settings
                currentIndex: appSettings.themeChoice
                onActivated: index => {
                    appSettings.themeChoice = index
                }
            }
        }

        // URL Input
        TextField {
            id: urlInput
            Layout.fillWidth: true
            placeholderText: "Paste video/audio link here..."
            enabled: !downloader.isBusy
            selectByMouse: true
            font.pixelSize: 14 // Increased from default (~13) to 14 or 15
        }

        // Destination Folder Row
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: pathField
                Layout.fillWidth: true
                text: downloader.downloadPath
                readOnly: true
                font.pixelSize: 13 // Increased from 11 to 13
            }

            Button {
                text: "Browse"
                flat: true
                enabled: !downloader.isBusy
                onClicked: folderDialog.open()
            }
        }

        // Quality Selectors
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Label { text: "Video Quality"; font.pixelSize: 11; opacity: 0.7 }
                ComboBox {
                    id: videoQualityCombo
                    Layout.fillWidth: true
                    enabled: !downloader.isBusy
                    model: ["Best Available", "2160p", "1440p", "1080p", "720p", "480p"]
                    currentIndex: 3
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Label { text: "Audio Bitrate"; font.pixelSize: 11; opacity: 0.7 }
                ComboBox {
                    id: audioQualityCombo
                    Layout.fillWidth: true
                    enabled: !downloader.isBusy
                    textRole: "text"
                    valueRole: "value"
                    model: [
                        { text: "Best (VBR)", value: "0" },
                        { text: "320 kbps",   value: "320k" },
                        { text: "192 kbps",   value: "192k" },
                        { text: "128 kbps",   value: "128k" }
                    ]
                    currentIndex: 0
                }
            }
        }

        // Action Buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: "Download MP4"
                highlighted: true
                Layout.fillWidth: true
                enabled: !downloader.isBusy && urlInput.text.trim().length > 0
                onClicked: downloader.downloadVideo(urlInput.text.trim(), videoQualityCombo.currentText)
            }

            Button {
                text: "Download MP3"
                highlighted: true
                Layout.fillWidth: true
                enabled: !downloader.isBusy && urlInput.text.trim().length > 0
                onClicked: downloader.downloadAudio(urlInput.text.trim(), audioQualityCombo.currentValue)
            }
        }

        Button {
            text: "Open Folder"
            flat: true
            Layout.alignment: Qt.AlignHCenter
            onClicked: downloader.openDownloadFolder()
        }

        Item { Layout.fillHeight: true }

        // Progress Status Card
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            ProgressBar {
                Layout.fillWidth: true
                from: 0.0
                to: 1.0
                value: downloader.progress
                indeterminate: downloader.isBusy && downloader.progress === 0.0
            }

            Label {
                text: downloader.status
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                font.pixelSize: 12
                color: {
                    if (downloader.status.indexOf("Complete") !== -1) return "#4caf50"
                    if (downloader.status.indexOf("failed") !== -1 ||
                        downloader.status.indexOf("Error") !== -1) return "#f44336"
                    if (downloader.isBusy) return Material.accent
                    return isDarkTheme ? "#9e9e9e" : "#616161"
                }
            }
        }
    }
}