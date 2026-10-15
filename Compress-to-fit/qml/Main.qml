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


    RowLayout {
        anchors.fill: parent

        //The file receiver
        FileEntry {
            id: fileEntry

            onReceivedPath: compressorSettings.visible = true

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: window.height / 1.7
            Layout.maximumHeight: window.height / 1.4
            Layout.preferredWidth: window.width * 0.35
            Layout.maximumWidth: window.width * 0.4
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            Layout.margins: 20
        }

        //initially non-visible settings for the compressor
        SettingsPanel {
            id: compressorSettings

            visible: false

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: window.width * 0.5
            Layout.maximumWidth: window.width * 0.6
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            Layout.margins: 20
        }

    }
}


