/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

/**
 * This component is intended to be used as root item for
 * KCMs with arbitrary content. Often a Kirigami.FormLayout
 * is used as main element.
 * It is possible to specify a header and footer component.
 * @code
 * import org.kde.kcmutils as KCMUtils
 * import org.kde.kirigami as Kirigami
 *
 * KCMUtils.SimpleKCM {
 *     Kirigami.FormLayout {
 *        TextField {
 *           Kirigami.FormData.label: "Label:"
 *        }
 *        TextField {
 *           Kirigami.FormData.label: "Label:"
 *        }
 *     }
 *     footer: Item {...}
 * }
 * @endcode
 * @inherits org.kde.kirigami.ScrollablePage
 */
Kirigami.ScrollablePage {
    id: root

    readonly property int margins: 6 // Layout_ChildMarginWidth from Breeze

    /**
     * extraFooterTopPadding: bool
     * @deprecated unused
     * Default: false
     */
    property bool extraFooterTopPadding: false

    /**
     * headerPaddingEnabled: bool
     * Whether the contents of the header will have automatic padding around it.
     * Should be disabled when using an InlineMessage or custom content item in
     * the header that's intended to touch the window edges.
     * Default: false
     */
    property bool headerPaddingEnabled: false

    property bool __flickableOverflows: flickable.contentHeight + flickable.topMargin + flickable.bottomMargin > flickable.height

    // Context properties are not reliable
    title: (typeof kcm !== "undefined") ? kcm.name : ""

    // Make pages fill the whole view by default
    Kirigami.ColumnView.fillWidth: true

    property bool sidebarMode: false

    topPadding: margins
    leftPadding: margins
    rightPadding: margins
    bottomPadding: margins

    header: QQC2.Control {
        id: headerParent

        readonly property bool contentVisible: contentItem && contentItem.visible && contentItem.implicitHeight

        height: contentVisible ? implicitHeight : 0
        padding: root.headerPaddingEnabled ? root.margins : 0

        // When the header is visible, we need to add a line below to separate
        // it from the view
        Kirigami.Separator {
            z: 999
            anchors {
                left: parent.left
                right: parent.right
                top: parent.bottom
            }
            visible: headerParent.contentVisible
        }
    }

    function __swapContentIntoContainer(property: string, container: Item): void {
        const content = this[property];

        if (content && content !== container) {
            // Revert the effect of repeated onHeaderChanged invocations
            // during initialization in Page super-type.
            content.anchors.top = undefined;

            this[property] = container;
            container.contentItem = content;
        }
    }

    function __adoptOverlaySheets(): void {
        // Search overlaysheets in contentItem, parent to root if found
        for (const object of contentItem.data) {
            if (object instanceof Kirigami.OverlaySheet) {
                if (object.parent === null) {
                    object.parent = this;
                }
                data.push(object);
            }
        }
    }

    Component.onCompleted: {
        __swapContentIntoContainer("header", headerParent);
        __adoptOverlaySheets();
    }
}
