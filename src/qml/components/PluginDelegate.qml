/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami 2 as Kirigami
import org.kde.kcmutils as KCM

/// @since 6.0, this got renamed from KPluginDelegate to PluginDelegate
// Not using Kirigami.CheckableListItem despite having a checkbox because we
// need the checkbox to be highlighted by KCM.SettingHighlighter, and
// CheckableListItem doesn't have that built in.
Kirigami.BasicListItem {
    id: listItem

    property list<QQC2.Action> additionalActions

    signal configTriggered()

    leading: QQC2.CheckBox {
        id: checkbox

        checked: model.enabled

        onToggled: model.enabled = checked

        KCM.SettingHighlighter {
            highlight: checkbox.checked !== model.enabledByDefault
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
    readonly property Kirigami.Action __infoAction: Kirigami.Action {
        id: infoAction

        icon.name: "dialog-information"
        text: i18nc("@info:tooltip", "About")
        displayHint: Kirigami.DisplayHint.IconOnly
        onTriggered: {
            const aboutDialog = listItem.ListView.view.__aboutDialog
            aboutDialog.metaDataInfo = model.metaData
            aboutDialog.open()
        }
    }

    readonly property Kirigami.Action __configureAction: Kirigami.Action {
        id: configureAction

        visible: model.config.isValid
        enabled: model.enabled
        icon.name: "configure"
        text: i18nc("@info:tooltip", "Configure…")
        displayHint: Kirigami.DisplayHint.IconOnly
        onTriggered: listItem.configTriggered()
    }

    trailing: Kirigami.ActionToolBar {
        actions: [infoAction, configureAction, ...listItem.additionalActions]
    }
}
