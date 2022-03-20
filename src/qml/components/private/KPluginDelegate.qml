/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcm 1.5 as KCM

Kirigami.SwipeListItem {
    id: listItem
    hoverEnabled: true
    onClicked: {
        if (view.currentIndex == index) {
            // Collapse list item
            view.currentIndex = -1;
        } else {
            // Expand list item
            view.currentIndex = index;
        }
    }
    contentItem: RowLayout {
        id: row

        QQC2.CheckBox {
            checkState: model.enabled ? Qt.Checked : Qt.Unchecked;

            onToggled: model.enabled = checkState

            KCM.SettingHighlighter {
                highlight: parent.checked !== model.enabledByDefault
            }
        }

        ColumnLayout {
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
            spacing: 0

            Kirigami.Heading {
                Layout.fillWidth: true

                level: 5
                text: model.name
                wrapMode: Text.Wrap
            }

            QQC2.Label {
                Layout.fillWidth: true

                text: model.description
                opacity: listItem.hovered ? 0.8 : 0.6
                wrapMode: Text.Wrap
            }

            QQC2.Label {
                id: aboutItem

                Layout.fillWidth: true

                text: i18n("Author: %1\nLicense: %2", "dummy author", "dummy license")
                opacity: listItem.hovered ? 0.8 : 0.6
                visible: view.currentIndex === index
                wrapMode: Text.Wrap
            }
        }
    }
    actions: [
        Kirigami.Action {
            visible: model.config.isValid
            enabled: model.enabled
            icon.name: "configure"
            tooltip: i18nc("@info:tooltip", "Configure...")
            onTriggered: kcm.configure(model.config, this)
        }
    ]
}
