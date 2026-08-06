import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore
import QtQuick.Layouts

Item {
    property string source

    RowLayout {
        anchors.fill: parent
        spacing: 8

        Text {
            id: textPath
            Layout.fillWidth: true
            text: fileDialog.selectedFile
            color: palette.text
            font.pixelSize: 20
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