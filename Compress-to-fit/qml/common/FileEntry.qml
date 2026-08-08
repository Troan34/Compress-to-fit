import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtCore
import QtQuick.Layouts

Item {
    property string source

    ColumnLayout {
        anchors.fill: parent
        spacing: 8


        DropArea {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.preferredHeight: 150
            Layout.preferredWidth: 150

            onEntered: (drag) => {
                dropAreaRect.color = "gray";
                drag.accept (Qt.LinkAction);
            }
            onDropped: (drop) => {
                console.log(drop.urls);
                dropAreaRect.color = Qt.rgba(palette.dark.r, palette.dark.g, palette.dark.b, palette.dark.a * 0.8);
            }
            onExited: {
                dropAreaRect.color = Qt.rgba(palette.dark.r, palette.dark.g, palette.dark.b, palette.dark.a / 2);
            }

            Rectangle {
                id: dropAreaRect
                anchors.fill: parent
                radius: 10

                color: Qt.rgba(palette.dark.r, palette.dark.g, palette.dark.b, palette.dark.a / 2)

                Text {
                    anchors.centerIn: parent
                    text: "Drop files here"
                }
            }
        }

        Button {
            text: qsTr("Choose file to process...")
            icon.name: "folder"
            onClicked: fileDialog.open()
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }
    }


    FileDialog {
        id: fileDialog
        currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
        onAccepted: compressor_conf.pathIn = selectedFile
    }
}