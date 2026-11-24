import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore

ColumnLayout {
    id: panel
    spacing: 14

    property bool isDarkTheme: false

    Settings {
        id: downloadSettings
        category: "Download"
        property string folder: ""
    }

    Component.onCompleted: {
        if (downloadSettings.folder.length > 0)
            downloadPanel.downloadPath = downloadSettings.folder
    }

    FolderDialog {
        id: folderDialog
        title: "Select Download Location"
        currentFolder: downloadPanel.downloadFolderUrl
        onAccepted: {
            downloadPanel.downloadPath = selectedFolder
            downloadSettings.folder = downloadPanel.downloadPath
        }
    }

    // URL Input
    TextField {
        id: urlInput
        Layout.fillWidth: true
        placeholderText: "Paste video/audio link here..."
        enabled: !downloadPanel.isBusy
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
            text: downloadPanel.downloadPath
            readOnly: true
            font.pixelSize: 13 // Increased from 11 to 13
        }

        Button {
            text: "Browse"
            flat: true
            enabled: !downloadPanel.isBusy
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
                enabled: !downloadPanel.isBusy
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
                enabled: !downloadPanel.isBusy
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
            enabled: !downloadPanel.isBusy && urlInput.text.trim().length > 0
            onClicked: downloadPanel.downloadVideo(urlInput.text.trim(), videoQualityCombo.currentText, pauseCheck.checked)
        }

        Button {
            text: "Download MP3"
            highlighted: true
            Layout.fillWidth: true
            enabled: !downloadPanel.isBusy && urlInput.text.trim().length > 0
            onClicked: downloadPanel.downloadAudio(urlInput.text.trim(), audioQualityCombo.currentValue, pauseCheck.checked)
        }
    }

    // Open Folder stays centred; the checkbox and Cancel sit at the sides of its row
    Item {
        Layout.fillWidth: true
        implicitHeight: openFolderButton.implicitHeight

        CheckBox {
            id: pauseCheck
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text: "Pause between videos"
            enabled: !downloadPanel.isBusy
        }

        Button {
            id: openFolderButton
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Open Folder"
            flat: true
            onClicked: downloadPanel.openDownloadFolder()
        }

        Button {
            anchors.right: parent.right
            text: "Cancel"
            flat: true
            visible: downloadPanel.isBusy
            onClicked: downloadPanel.cancel()
        }
    }

    Item { Layout.fillHeight: true }

    StatusCard {
        Layout.fillWidth: true
        isDarkTheme: panel.isDarkTheme
    }
}
