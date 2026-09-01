/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*/
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtCore
import QtQuick.Layouts

Item {
    id: rootDropArea
    implicitWidth: 200
    implicitHeight: 200

    signal receivedDrop()

    function receiveDrop(drop) {
        receiveUrls(drop.urls)
        receivedDrop()
    }

    function receiveUrls(urls) {
        if (urls.length > 1)
        {
            filePathInfo.text = qsTr("Multiple files selected")
        }
        else
        {
            filePathInfo.text = fileSystem.fileName(urls[0])
        }

        dropAreaCont.updateBackgroundSVGFromUrls(urls)

        let size = 0
        for (let i = 0; i < urls.length; i++)
            size += fileSystem.fileSize(urls[i])

        sizeInfo.text = fileSystem.toUnit(size)

        dropAreaCont.color = Qt.rgba(palette.dark.r, palette.dark.g, palette.dark.b, palette.dark.a * 0.3)
        dropAreaText.text = ""

        compressor_conf.pathsIn = urls
    }

    Rectangle { //Background square
        id: dropAreaCont
        anchors.fill: parent
        radius: 15
        color: Qt.rgba(palette.dark.r, palette.dark.g, palette.dark.b, palette.dark.a / 2)

        /**
         *  @brief Given a list of urls, this function decides the icon to show
         *  @param urls The list of urls
         */
        function updateBackgroundSVGFromUrls(urls) {
            if (urls.length > 1)
            {
                backgroundImage.source = "qrc:/qt/qml/CompressToFit/qml/icons/files_24dp_E3E3E3_FILL0_wght400_GRAD0_opsz24.svg"
                return
            }
            let path = urls[0]
            if (fileSystem.extension(path) === ".png" ||
                fileSystem.extension(path) === ".jpg" ||
                fileSystem.extension(path) === ".jpeg"||
                fileSystem.extension(path) === ".bmp" ||
                fileSystem.extension(path) === ".tiff")
            {
                backgroundImage.source = "qrc:/qt/qml/CompressToFit/qml/icons/photo_24dp_E3E3E3_FILL1_wght400_GRAD0_opsz24.svg"
            }
            else if (
                fileSystem.extension(path) === ".pdf" ||
                fileSystem.extension(path) === ".doc" ||
                fileSystem.extension(path) === ".docx"||
                fileSystem.extension(path) === ".txt" ||
                fileSystem.extension(path) === ".rtf")
            {
                backgroundImage.source = "qrc:/qt/qml/CompressToFit/qml/icons/docs_24dp_E3E3E3_FILL1_wght400_GRAD0_opsz24.svg"
            }
            else if (fileSystem.isDirectory(path)) {
                backgroundImage.source = "qrc:/qt/qml/CompressToFit/qml/icons/folder_24dp_E3E3E3_FILL1_wght400_GRAD0_opsz24.svg"
            }
        }


        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            //File size text
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignVCenter| Qt.AlignHCenter
                Layout.preferredHeight: dropAreaCont.width * 0.2

                //color: "red"
                Text {
                    id: sizeInfo
                    anchors.fill: parent
                    wrapMode: Text.Wrap
                    verticalAlignment: Text.AlignBottom
                    horizontalAlignment: Text.AlignHCenter

                    color: palette.text
                    font.pointSize: 18
                    font.weight: 700
                }
            }

            //Icon
            Item {
                id: backgroundImageContainer
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.minimumWidth: 50
                Layout.minimumHeight: 50
                Layout.preferredWidth: dropAreaCont.width * 0.6
                Layout.preferredHeight: Layout.preferredWidth
                Layout.maximumWidth: dropAreaCont.width * 0.6
                Layout.maximumHeight: Layout.maximumWidth
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

                //color: "green"

                Image {
                    id: backgroundImage
                    anchors.fill: parent

                    sourceSize.width: width
                    sourceSize.height: height

                    fillMode: Image.PreserveAspectFit

                }
            }

            //File name
            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredHeight: dropAreaCont.width * 0.2

                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter

                //color: "blue"

                Text {
                    id: filePathInfo
                    anchors.fill: parent
                    wrapMode: Text.Wrap
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter

                    color: palette.text

                    fontSizeMode: Text.Fit
                    font.pointSize: 12
                    minimumPointSize: 8

                }
            }


        }


        DropArea {
            anchors.fill: parent

            onEntered: (drag) => {
                dropAreaCont.color = "gray";
                drag.accept(Qt.LinkAction);
            }
            onDropped: (drop) => {
                rootDropArea.receiveDrop(drop)
            }

            onExited: {
                dropAreaCont.color = Qt.rgba(palette.dark.r, palette.dark.g, palette.dark.b, palette.dark.a / 2)
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