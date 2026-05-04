/*
 SPDX-FileCopyrightText: Akseli Lahtinen 2026 <akselmo@akselmo.dev>

 SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Loader {
    id: root
    active: typeof kcm !== "undefined"

    property bool highlight: !model.isDefault ?? false

    sourceComponent: Kirigami.Badge {
        Layout.alignment: Qt.AlignVCenter
        Layout.preferredWidth: Kirigami.Units.largeSpacing
        Layout.preferredHeight: Kirigami.Units.largeSpacing

        visible: kcm.defaultsIndicatorsVisible && root.highlight
        type: Kirigami.Badge.Type.Warning
    }
}