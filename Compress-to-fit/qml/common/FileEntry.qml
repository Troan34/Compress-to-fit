import QtQuick
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtCore
import QtQuick.Layouts
import CompressToFit

Item {

    ColumnLayout {
        anchors.fill: parent
        spacing: 8
        Layout.margins: 10

        InteractiveDropArea {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 200
            Layout.preferredWidth: Layout.preferredHeight
            Layout.minimumHeight: 100
            Layout.minimumWidth: Layout.minimumHeight
            Layout.maximumHeight: 300
            Layout.maximumWidth: Layout.maximumHeight
        }

        Button {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

            /*
            background: Rectangle {
                color: parent.hovered ? "lightblue" : palette.button
            }
            */

            text: qsTr("Choose files to process...")
            icon.name: "folder"
            onClicked: fileDialog.open()
        }
    }


    FileDialog {
        id: fileDialog
        currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
        onAccepted: compressor_conf.pathsIn = selectedFiles
    }
}