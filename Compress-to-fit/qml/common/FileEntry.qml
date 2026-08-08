import QtQuick
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtCore
import QtQuick.Layouts

Item {
    property string source

    ColumnLayout {
        anchors.fill: parent
        spacing: 8
        Layout.margins: 10

        Rectangle {
            id: dropAreaRect
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 200
            Layout.preferredWidth: Layout.preferredHeight
            Layout.minimumHeight: 100
            Layout.minimumWidth: Layout.minimumHeight
            Layout.maximumHeight: 300
            Layout.maximumWidth: Layout.maximumHeight


            radius: 15

            color: Qt.rgba(palette.dark.r, palette.dark.g, palette.dark.b, palette.dark.a / 2)

            DropArea {
                anchors.fill: parent

                onEntered: (drag) => {
                    dropAreaRect.color = "gray";
                    drag.accept(Qt.LinkAction);
                }
                onDropped: (drop) => {
                    console.log(drop.urls);
                    dropAreaRect.color = Qt.rgba(palette.dark.r, palette.dark.g, palette.dark.b, palette.dark.a * 0.8);
                }
                onExited: {
                    dropAreaRect.color = Qt.rgba(palette.dark.r, palette.dark.g, palette.dark.b, palette.dark.a / 2);
                }

                Text {
                    anchors.centerIn: parent
                    color: palette.text
                    font.pointSize: 15
                    text: "Drop files here"
                }
            }

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
        onAccepted: compressor_conf.pathIn = selectedFile
    }
}