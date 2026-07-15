import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore

ApplicationWindow {
    width: 640
    height: 480
    visible: true
    title: "Compress To Fit"


    component FileEntry: Item {
        property string source
        Button {
            id: folderButton

            anchors.centerIn: parent

            text: "Choose file to process..."
            icon.name: "folder"
            onClicked: fileDialog.open()

        }

        FileDialog {
            id: fileDialog
            currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
            onAccepted: image.source = selectedFile
        }
    }

    FileEntry {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 20
    }
    Image {
        id: image
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
    }


}


