/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*/
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
    Material.theme: Material.System

    visible: true
    title: "Compress To Fit"


    GridLayout {
        anchors.fill: parent

        states: [
            State {
                when: width < minimumWidth * 2.2
                PropertyChanges { target: compressorSettings; Layout.row: 1 }
                PropertyChanges { target: compressorSettings; Layout.column: 0 }
            },
            State {
                when: width >= minimumWidth * 2.2
                PropertyChanges { target: compressorSettings; Layout.row: 0 }
                PropertyChanges { target: compressorSettings; Layout.column: 1 }
            }
        ]

        //The file receiver
        FileEntry {
            id: fileEntry

            onReceivedPath: compressorSettings.visible = true

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: window.height * 0.4
            Layout.preferredWidth: window.width * 0.4
            Layout.maximumHeight: window.height * 0.7
            Layout.maximumWidth: window.width * 0.7
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            Layout.rightMargin: 10
            Layout.leftMargin: 10
        }

        //initially non-visible settings for the compressor
        SettingsPanel {
            id: compressorSettings

            visible: false

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: window.width * 0.5
            Layout.preferredHeight: window.height * 0.5
            Layout.maximumWidth: window.width * 0.6
            Layout.maximumHeight: window.width * 0.8
            Layout.rightMargin: 10
            Layout.leftMargin: 10
        }

    }
}


