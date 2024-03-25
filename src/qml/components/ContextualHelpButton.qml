/*
   SPDX-FileCopyrightText: 2020 Felix Ernst <fe.a.ernst@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

QQC2.Button {
    id: root

    icon.name: "help-contextual"
    flat: true
    property alias toolTipText: toolTip.text
    property bool toolTipVisible: false

    onReleased: {
        toolTip.delay = toolTipVisible ? Kirigami.Units.toolTipDelay : 0;
        toolTipVisible = !toolTipVisible;
    }
    onActiveFocusChanged: {
        toolTip.delay = Kirigami.Units.toolTipDelay;
        toolTipVisible = false;
    }
    Layout.maximumHeight: parent?.height ?? -1
    QQC2.ToolTip {
        id: toolTip
        implicitWidth: Math.min(21 * Kirigami.Units.gridUnit, root.Window.width) // Wikipedia says anything between 45 and 75 characters per line is acceptable. 21 * Kirigami.Units.gridUnit feels right.
        visible: root.hovered || root.toolTipVisible
        onVisibleChanged: {
            if (!visible && root.toolTipVisible) {
                root.toolTipVisible = false;
                delay = Kirigami.Units.toolTipDelay;
            }
        }
        timeout: -1
    }
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.WhatsThisCursor
        acceptedButtons: Qt.NoButton
    }
    Accessible.name: i18ndc("kcmutils6", "@action:button", "Show Contextual Help")
}

