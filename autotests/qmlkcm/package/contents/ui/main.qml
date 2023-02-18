/**
 * SPDX-FileCopyrightText: Year Author <author@domanin.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls

import org.kde.kirigami 2.7 as Kirigami
import org.kde.kcm 1.2
import org.kde.kcmutils 1.0

SimpleKCM {
    ConfigModule.buttons: ConfigModule.NoAdditionalButton // Just to show that setting the buttons works
    Component.onCompleted: console.log(ConfigModule.buttons)
    Controls.Label {
        text: i18n("Configure Time")
    }
}
