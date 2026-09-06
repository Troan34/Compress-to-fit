/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*   Provides a panel like component that groups compression settings
*/

import QtQuick
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtCore
import QtQuick.Layouts

Item {
    id: rootPanel

    //overarching layout, i.e. a first compressor row, a second preset row et cetera
    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        Item { //I just found out about this method of padding and I love its ingenuity
            Layout.fillHeight: true
        }

        //models
        RowLayout {
            Text {
                text: qsTr("Compressor")

                color: palette.text
                fontSizeMode: Text.Fit
                font.pointSize: 12
                minimumPointSize: 8
            }
            RowLayout {
                //LZ77 button
                RadioButton {
                    checked: true
                    text: "LZ77"
                    id: lz77Button

                    onCheckedChanged: checked ? compressor_conf.compressor = "LZ77" : compressor_conf.compressor = ""

                    Popup {
                        x: lz77Button.width / 2 - width / 2
                        y: -height - 5

                        width: 200
                        padding: 8

                        visible: lz77Button.hovered

                        contentItem: Text {
                            text: qsTr(
                                "Length-distance based algorithm, good for repetitive " +
                                "patterns of data. Slow-ish compression and fast decompression."
                            )
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

                //LZ78 button
                RadioButton {
                    text: "LZ78"
                    id: lz78Button

                    onCheckedChanged: checked ? compressor_conf.compressor = "LZ78" : compressor_conf.compressor = "LZ78"

                    Popup {
                        width: 200

                        visible: lz78Button.hovered

                        x: lz78Button.width / 2 - width / 2
                        y: -height - 5

                        contentItem: Text {
                            text: qsTr( "Dictionary based algorithm, good for repetitive patterns of data. " +
                                "Slow-ish compression and fast decompression. " +
                                "More advanced and faster than LZ77. "
                            )

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
            }
        }

        //Preset
        RowLayout {
            id: compressorOptions

            Text {
                text: qsTr("Compression preset")
                color: palette.text

                fontSizeMode: Text.Fit
                font.pointSize: 12
                minimumPointSize: 8
            }

            //Sets the preset to the backend and updates the frontend
            function setPreset(value)
            {
                slider.value = Math.round(value)
                textValue.text = String(Math.round(value))
                compressor_conf.compressorPreset = value
            }

            Slider {
                id: slider

                Layout.preferredWidth: 200
                Layout.minimumWidth: 100
                Layout.fillWidth: true


                snapMode: Slider.SnapOnRelease
                from: 0
                value: 4
                to: 9
                stepSize: 1

                onValueChanged: { compressorOptions.setPreset(value) }
            }

            TextField {
                id: textValue
                color: palette.text

                Layout.minimumWidth: 40
                Layout.minimumHeight: Layout.minimumWidth
                Layout.preferredWidth: 40
                Layout.preferredHeight: Layout.preferredWidth
                Layout.maximumWidth: 40
                Layout.maximumHeight: Layout.maximumWidth

                text: "4"
                onTextChanged: { compressorOptions.setPreset(Number(text)) }
                validator: IntValidator { bottom: slider.from; top: slider.to; }
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredWidth: 10
            }
        }


        RowLayout {
            id: misc
            Text {
                text: qsTr("Misc options")
                color: palette.text

                fontSizeMode: Text.Fit
                font.pointSize: 12
                minimumPointSize: 8
            }

            RowLayout {
                //forceCompression
                CheckBox {
                    text: qsTr("force compression")
                    onCheckedChanged: checked ? compressor_conf.forceCompression = true : compressor_conf.forceCompression = false
                }

                CheckBox {
                    text: qsTr("delete input on completion")
                    onCheckedChanged: checked ? compressor_conf.deleteInput = true : compressor_conf.deleteInput = false
                }
            }

            TextField {
                color: palette.text
                placeholderText: qsTr("Choose number of files the output will be split into")

                Layout.minimumWidth: 40
                Layout.minimumHeight: Layout.minimumWidth
                Layout.preferredWidth: 40
                Layout.preferredHeight: Layout.preferredWidth

                onTextChanged: { compressor_conf.numberOfFiles = Number(text) }
                validator: IntValidator { bottom: 1; top: 1000; }
            }
        }


        Item {
            Layout.fillHeight: true
        }
    }
}