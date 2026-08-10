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
    minimumWidth: 360
    minimumHeight: 240

    visible: true
    title: "Compress To Fit"

    ColumnLayout {
        anchors.fill: parent

        FileEntry {
            id: fileEntry
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

    }
}


