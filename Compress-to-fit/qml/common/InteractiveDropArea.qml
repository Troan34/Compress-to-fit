import QtQuick
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtCore
import QtQuick.Layouts

Item {
    implicitWidth: 200
    implicitHeight: 200

    Rectangle {
        id: dropAreaRect
        anchors.fill: parent
        radius: 15

        color: Qt.rgba(palette.dark.r, palette.dark.g, palette.dark.b, palette.dark.a / 2)

        ColumnLayout {
            RowLayout {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignTop

                Item {
                    id: backgroundImageContainer
                    Layout.fillWidth: true
                    Layout.minimumWidth: 50
                    Layout.minimumHeight: 50
                    Layout.preferredWidth: dropAreaRect.width * 0.4
                    Layout.preferredHeight: Layout.preferredWidth
                    Layout.maximumWidth: dropAreaRect.width * 0.4
                    Layout.maximumHeight: Layout.maximumWidth
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    Image {
                        id: backgroundImage
                        anchors.fill: parent

                        sourceSize.width: width
                        sourceSize.height: height

                        fillMode: Image.PreserveAspectFit

                        function updateBackgroundSVGFromPath(path) {
                            if (file_helper.extension(path) === ".png" ||
                                file_helper.extension(path) === ".jpg" ||
                                file_helper.extension(path) === ".jpeg"||
                                file_helper.extension(path) === ".bmp" ||
                                file_helper.extension(path) === ".tiff")
                            {
                                backgroundImage.source = "qrc:/qt/qml/CompressToFit/qml/icons/photo_24dp_E3E3E3_FILL1_wght400_GRAD0_opsz24.svg"
                            }
                            else if (
                                file_helper.extension(path) === ".pdf" ||
                                file_helper.extension(path) === ".doc" ||
                                file_helper.extension(path) === ".docx"||
                                file_helper.extension(path) === ".txt" ||
                                file_helper.extension(path) === ".rtf"
                            )
                            {
                                backgroundImage.source = "qrc:/qt/qml/CompressToFit/qml/icons/docs_24dp_E3E3E3_FILL1_wght400_GRAD0_opsz24.svg"
                            }
                            else if (file_helper.isDirectory(path)) {
                                backgroundImage.source = "qrc:/qt/qml/CompressToFit/qml/icons/folder_24dp_E3E3E3_FILL1_wght400_GRAD0_opsz24.svg"
                            }
                        }

                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.maximumWidth: dropAreaRect.width * 0.5
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

                    Text {
                        id: filePathInfo
                        anchors.fill: parent

                        color: palette.text
                        font.pointSize: 12
                    }
                }
            }

            RowLayout {
                Text {

                }
            }
        }



        DropArea {
            anchors.fill: parent

            onEntered: (drag) => {
                dropAreaRect.color = "gray";
                drag.accept(Qt.LinkAction);
            }
            onDropped: (drop) => {
                console.log(drop.urls);

                if (drop.urls.length > 1)
                {
                    filePathInfo.text = qsTr("Multiple files selected");
                }
                else
                {
                    filePathInfo.text = file_helper.fileName(drop.urls[0]);
                }
                for (let i = 0; i < drop.urls.length; i++)
                {
                    backgroundImage.updateBackgroundSVGFromPath(drop.urls[i]);
                }

                dropAreaRect.color = Qt.rgba(palette.dark.r, palette.dark.g, palette.dark.b, palette.dark.a * 0.3);
                dropAreaText.text = "";
            }
            onExited: {
                dropAreaRect.color = Qt.rgba(palette.dark.r, palette.dark.g, palette.dark.b, palette.dark.a / 2);
            }

            Text {
                id: dropAreaText
                anchors.centerIn: parent
                color: palette.text
                font.pointSize: 15
                text: "Drop files here"
            }
        }
    }
}