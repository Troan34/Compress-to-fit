import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore
import QtQuick.Layouts

Item {
    property string source

    RowLayout {
        anchors.fill: parent
        
        TextArea {
            id: textPath
            placeholderText: qsTr("Insert the path of the file you want to elaborate.")
            text: fileDialog.selectedFile
            width: parent.width
        }

        Button {
            text: qsTr("Choose file to process...")
            icon.name: "folder"
            onClicked: fileDialog.open()
        }
    }


    FileDialog {
        id: fileDialog
        currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
        onAccepted: compressor_conf.pathIn = selectedFile
    }
}