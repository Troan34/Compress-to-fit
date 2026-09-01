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
            RadioButton {
                checked: true
                text: qsTr("LZ77")

                contentItem: Text {
                    text: parent.text
                    color: palette.text

                    opacity: enabled ? 1.0 : 0.3
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: parent.indicator.width + parent.spacing
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

            /**
             * Sets the preset to the backend and updates the frontend
             */
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

                onTextChanged: { compressorOptions.setPreset(Number(text)) }
                validator: IntValidator { bottom: slider.from; top: slider.to; }
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredWidth: 10
            }
        }


        Item {
            Layout.fillHeight: true
        }
    }
}