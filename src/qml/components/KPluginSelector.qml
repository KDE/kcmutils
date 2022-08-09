/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1

import org.kde.kirigami 2.19 as Kirigami

import org.kde.kcmutils.private 1.0 as KCMUtilsPrivate
import "private" as Private

/**
 * ListView for showing plugins with their info and configuration.
 * If extra butons should be added, a custom KPluginDelegate with the additionalActions
 * property should be defined.
 * 
 * @since 5.94
 */
ListView {
    id: pluginSelector
    // KPluginModel which contains the plugins that should be displayed
    required property QtObject sourceModel
    // Query that is typed into the search field. Ideally, this is part of the KCM header
    property var query

    clip: true

    // Don't select anything by default as selection is not used here
    currentIndex: -1

    model: KCMUtilsPrivate.ProxyModel {
        id: proxyModel
        model: pluginSelector.sourceModel
        query: pluginSelector.query ?? ""
    }

    delegate: KPluginDelegate {
    }

    section.property: "category"
    section.delegate: Kirigami.ListSectionHeader {
        width: pluginSelector.width
        text: section
    }

    Kirigami.OverlaySheet {
        id: internalAboutDialog
        parent: pluginSelector.parent
        property var metaDataInfo

        contentItem: Loader {
            active: internalAboutDialog.metaDataInfo !== undefined
            sourceComponent: ColumnLayout {
                Private.AboutPlugin {
                    metaData: internalAboutDialog.metaDataInfo
                    Layout.maximumWidth: Math.min(Kirigami.Units.gridUnit * 30, Math.round(pluginSelector.width * 0.8))
                }
            }
        }
    }
    // Only for internal usage in KPluginDelegate!
    property var __aboutDialog: internalAboutDialog

    Loader {
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.gridUnit * 8)
        active: pluginSelector.count === 0 && !startupTimer.running
        opacity: active && status === Loader.Ready ? 1 : 0
        visible: opacity > 0
        Behavior on opacity {
            OpacityAnimator {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        sourceComponent: Kirigami.PlaceholderMessage {
            icon.name: "edit-none"
            text: pluginSelector.query && pluginSelector.query.length > 0 ? i18n("No matches") : i18n("No plugins found")
        }
    }

    // The view can take a bit of time to initialize itself when the KCM first
    // loads, during which time count is 0, which would cause the placeholder
    // message to appear for a moment and then disappear. To prevent this, let's
    // suppress it appearing for a moment after the KCM loads.
    Timer {
        id: startupTimer
        interval: Kirigami.Units.humanMoment
        running: false
    }
    Component.onCompleted: {
        startupTimer.start()
    }
}
