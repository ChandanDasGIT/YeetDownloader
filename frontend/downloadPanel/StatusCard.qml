import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

ColumnLayout {
    spacing: 6

    property bool isDarkTheme: false

    ProgressBar {
        Layout.fillWidth: true
        from: 0.0
        to: 1.0
        value: downloadPanel.progress
        indeterminate: downloadPanel.isBusy && downloadPanel.progress === 0.0
    }

    Label {
        text: downloadPanel.status
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        font.pixelSize: 12
        color: {
            if (downloadPanel.status.indexOf("Complete") !== -1) return "#4caf50"
            if (downloadPanel.status.indexOf("failed") !== -1 ||
                downloadPanel.status.indexOf("Error") !== -1) return "#f44336"
            if (downloadPanel.isBusy) return Material.accent
            return isDarkTheme ? "#9e9e9e" : "#616161"
        }
    }
}
