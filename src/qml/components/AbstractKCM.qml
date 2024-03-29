/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

/**
 * This component is intended to be used as root item for
 * KCMs with arbitrary content. Unlike SimpleKCM this does NOT
 * provide a scrollable view, The developer will have to manage
 * their own scrollviews.
 * Most of the times SimpleKCM should be used instead
 * @code
 * import QtQuick
 * import QtQuick.Controls as QQC2
 * import QtQuick.Layouts
 * import org.kde.kcmutils as KCM
 *
 * KCM.AbstractKCM {
 *     RowLayout {
 *         QQC2.ScrollView {
 *             // ...
 *         }
 *         QQC2.ScrollView {
 *             // ...
 *         }
 *     }
 *     footer: QQC2.ToolBar {
 *         // ...
 *     }
 * }
 * @endcode
 * @inherits org.kde.kirigami.Page
 * @since 6.0
 */
Kirigami.Page {
    id: root

    readonly property int margins: 6 // Layout_ChildMarginWidth from Breeze

    /**
     * framedView: bool
     * Whether to use this component as the base of a "framed" KCM with an
     * inner scrollview that draws its own frame.
     * Default: true
     */
    property bool framedView: true

    /**
     * extraFooterTopPadding: bool
     * @deprecated unused
     * Default: false
     */
    property bool extraFooterTopPadding: false

    property bool sidebarMode: false

    function __itemVisible(item: Item): bool {
        return item !== null && item.visible && item.implicitHeight > 0;
    }

    function __headerContentVisible(): bool {
        return __itemVisible(headerParent.contentItem);
    }
    function __footerContentVisible(): bool {
        return __itemVisible(footerParent.contentItem);
    }

    // Deliberately not checking for __footerContentVisible because
    // we always want the footer line to be visible when the scrollview
    // doesn't have a frame of its own, because System Settings always
    // adds its own footer for the Apply, Help, and Defaults buttons
    function __headerSeparatorVisible(): bool {
        return !framedView && __headerContentVisible();
    }
    function __footerSeparatorVisible(): bool {
        return !framedView && extraFooterTopPadding;
    }

    title: (typeof kcm !== "undefined") ? kcm.name : ""

    // Make pages fill the whole view by default
    Kirigami.ColumnView.fillWidth: sidebarMode
                                   ? Kirigami.ColumnView.view
                                         && (Kirigami.ColumnView.view.width < Kirigami.Units.gridUnit * 36
                                             || Kirigami.ColumnView.index >= Kirigami.ColumnView.view.count - 1)
                                   : true

    padding: 0
    topPadding: framedView && !__headerContentVisible() ? margins : 0
    leftPadding: undefined
    rightPadding: undefined
    bottomPadding: framedView && !__footerContentVisible() ? margins : 0
    verticalPadding: undefined
    horizontalPadding: framedView ? margins : 0

    header: Kirigami.Padding {
        id: headerParent

        height: root.__headerContentVisible()
            ? undefined
            : (root.__headerSeparatorVisible()
                ? headerSeparator.implicitHeight
                : 0)

        padding: root.margins
        bottomPadding: root.__headerSeparatorVisible()
            ? verticalPadding + headerSeparator.implicitHeight
            : undefined

        // When the scrollview isn't drawing its own frame, we need to add a
        // line below the header (when visible) to separate it from the view
        Kirigami.Separator {
            id: headerSeparator
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            visible: root.__headerSeparatorVisible()
        }
    }

    // View background, shown when the scrollview isn't drawing its own frame
    Rectangle {
        anchors.fill: parent
        visible: !root.framedView
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        color: Kirigami.Theme.backgroundColor
    }

    footer: Kirigami.Padding {
        id: footerParent

        height: root.__footerContentVisible()
            ? undefined
            : (root.__footerSeparatorVisible()
                ? footerSeparator.implicitHeight
                : 0)

        padding: root.margins
        topPadding: root.__footerSeparatorVisible()
            ? verticalPadding + footerSeparator.implicitHeight
            : undefined

        // When the scrollview isn't drawing its own frame, we need to add a
        // line above the footer ourselves to separate it from the view
        Kirigami.Separator {
            id: footerSeparator
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            visible: root.__footerSeparatorVisible()
        }
    }

    function __swapContentIntoContainer(property: string, container: Item) {
        const content = this[property];

        if (content && content !== container) {
            // Revert the effect of repeated onHeaderChanged invocations
            // during initialization in Page super-type.
            content.anchors.top = undefined;

            this[property] = container;
            container.contentItem = content;
            container.visible = true;
        }
    }

    Component.onCompleted: {
        __swapContentIntoContainer("header", headerParent);
        __swapContentIntoContainer("footer", footerParent);

        //Search overlaysheets in contentItem, parent to root if found
        for (const obj of contentItem.data) {
            if (obj instanceof Kirigami.OverlaySheet) {
                if (!obj.parent) {
                    obj.parent = this;
                }
                data.push(obj);
            }
        }
    }
}
