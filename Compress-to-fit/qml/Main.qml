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
            anchors.fill: parent
            anchors.margins: 20
        }
        Layout.fillWidth: true
        Layout.preferredHeight: 100


    }

}


