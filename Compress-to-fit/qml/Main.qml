import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore
import QtQuick.Layouts
import CompressToFit

ApplicationWindow {
    width: 1280
    height: 720
    visible: true
    title: "Compress To Fit"

    ColumnLayout {
        anchors.fill: parent

        FileEntry {
            id: fileEntry
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        Rectangle {
            color: "slategray"
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        Rectangle {
            color: "lightskyblue"
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}


