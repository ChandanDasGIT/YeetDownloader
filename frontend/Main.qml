import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtCore // Required for persistent Settings
import YeetDownloader

ApplicationWindow {
    id: root
    visible: true
    width: 540
    height: 600
    minimumWidth: 500
    // Never smaller than the content, so the status card can't be cut off
    minimumHeight: content.implicitHeight + 2 * content.anchors.margins
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

    ColumnLayout {
        id: content
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

        DownloadPanel {
            Layout.fillWidth: true
            Layout.fillHeight: true
            isDarkTheme: root.isDarkTheme
        }
    }
}
