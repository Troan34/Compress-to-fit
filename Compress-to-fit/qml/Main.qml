import QtQuick
import QtQuick.Window
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtCore
import QtQuick.Layouts
import QtQml
import CompressToFit

ApplicationWindow {
    id: window
    width: 1280
    height: 720
    minimumWidth: 360
    minimumHeight: 240

    visible: true
    title: "Compress To Fit"

    color: palette.window

    ColumnLayout {
        anchors.fill: parent

        FileEntry {
            id: fileEntry
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: window.height / 2
            Layout.maximumHeight: window.height / 2
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
        }

        Button {
            id: nextButton
            //TODO: Maybe customize the Animation when the button is added
            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

            visible: false
            text: qsTr("Next")

            Connections {
                target: compressor_conf
                function onPathsInChanged() {
                    nextButton.visible = true
                }
            }
        }

    }
}


