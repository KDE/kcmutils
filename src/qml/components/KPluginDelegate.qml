/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Controls 2.4 as Controls

import QtQuick.Layouts 1.1

import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcm 1.5 as KCM

/// @since 5.94
// Not using Kirigami.CheckableListItem despite having a checkbox because we
// need the checkbox to be highlighted by KCM.SettingHighlighter, and
// CheckableListItem doesn't have that built in.
Kirigami.BasicListItem {
    id: listItem

    property list<QQC2.Action> additionalActions
    signal configTriggered()

    leading: QQC2.CheckBox {
        checkState: model.enabled ? Qt.Checked : Qt.Unchecked;

        onToggled: model.enabled = checkState

        KCM.SettingHighlighter {
            highlight: parent.checked !== model.enabledByDefault
        }
    }

    icon.name: model.icon
    label: model.name
    subtitle: model.description

    // Don't need highlight or hover effects; there is no concept of selection here
    hoverEnabled: false

    // Some items don't have subtitles and we want everything to have a consistent height
    reserveSpaceForSubtitle: true

    // Take care of displaying the actions
    property var infoAction: Kirigami.Action {
        icon.name: "dialog-information"
        tooltip: i18nc("@info:tooltip", "About")
        onTriggered: {
            const aboutDialog = listItem.parent.parent.__aboutDialog
            aboutDialog.metaDataInfo = model.metaData
            aboutDialog.open()
        }
    }
    property var configureAction: Kirigami.Action {
        visible: model.config.isValid
        enabled: model.enabled
        icon.name: "configure"
        tooltip: i18nc("@info:tooltip", "Configure...")
        onTriggered: listItem.configTriggered()
    }

    // Put this in an intermediary property so that we can append to the list
    property var __allActions: []
    Component.onCompleted:  {
        const tmp = [];
        tmp.push(infoAction)
        tmp.push(configureAction)
        for (let i = 0; i < additionalActions.length; i++) {
            tmp.push(additionalActions[i])
        }
        __allActions = tmp
    }
    trailing: Kirigami.ActionToolBar {
        actions: __allActions
    }
}
