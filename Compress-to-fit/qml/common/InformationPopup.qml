/*
*   Copyright (C) 2026 Roan Bukaci
*   SPDX-License-Identifier: GPL-3.0
*   Spawn a popup that contains information.
*/
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtCore
import QtQuick.Layouts


Popup {
    id: popup
    width: 200
    height: 200
    focus: true
    margins: -1

    property int distance
    property real restX//position when closed, set by the parent
    onRestXChanged: if (!visible) x = restX

    enter: Transition {
        NumberAnimation {
            property: "x"
            from: popup.restX
            to: popup.restX - popup.distance
            duration: 300
            easing.type: Easing.OutCubic
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "x"
            from: popup.restX - popup.distance
            to: popup.restX
            duration: 200
            easing.type: Easing.InCubic
        }
    }

    background: Rectangle {
        color: Qt.rgba(palette.dark.r, palette.dark.g, palette.dark.b, palette.dark.a * 0.9)
        radius: 25
    }
}

