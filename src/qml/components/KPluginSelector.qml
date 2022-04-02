/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.10 as Kirigami

import org.kde.kcmutils.private 1.0 as KCMUtilsPrivate
import "private" as Private

ListView {
    id: pluginSelector;
    property QtObject sourceModel;

    header: Kirigami.SearchField {
        id: searchField
        width: pluginSelector.width
        onTextChanged: function (text) {
            proxyModel.query = searchField.text;
                searchField.forceActiveFocus();
        }
    }


    clip: true

    model: KCMUtilsPrivate.ProxyModel {
        id: proxyModel
        model: pluginSelector.sourceModel
    }

    delegate: KPluginDelegate {
        aboutDialog: pluginSelector.aboutDialog
    }

    section.property: "category"
    section.delegate: Kirigami.ListSectionHeader {
        width: pluginSelector.width
        text: section
    }

    Kirigami.OverlaySheet {
        id: internalAboutDialog
        property var metaDataInfo
        Kirigami.Theme.inherit: true

        contentItem: Loader {
            active: aboutDialog.metaDataInfo !== undefined
            sourceComponent: Private.AboutPlugin {
                metaData: aboutDialog.metaDataInfo
            }
        }
    }
    property var aboutDialog: internalAboutDialog

}
