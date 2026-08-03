import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore
import CompressToFit

ApplicationWindow {
    width: 1280
    height: 720
    visible: true
    title: "Compress To Fit"

    FileEntry {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 20
        width: parent.width
    }

}


