/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2

QQC2.Menu {
    id: menu

    function trigger() {
        menu.parent.clicked()
        if (menu.parent.actions.length > 0) {
            menu.popup(menu.parent, thumbnail.x, thumbnail.y + thumbnail.height)
        }
    }

    onClosed: menu.parent.forceActiveFocus()

    Repeater {
        model: menu.parent.actions
        delegate: QQC2.MenuItem {
            text: modelData.text || modelData.tooltip
            icon.name: modelData.icon.name
            enabled: modelData.enabled
            visible: modelData.visible

            onTriggered: modelData.trigger()
        }
    }
}
