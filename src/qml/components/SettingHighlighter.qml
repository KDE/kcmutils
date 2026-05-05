/*
    SPDX-FileCopyrightText: 2020 Kevin Ottens <kevin.ottens@enioka.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david.redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.kcmutils.private as KCMUtilsPrivate
import org.kde.kirigami as Kirigami

/*!
   \qmltype SettingHighlighter
   \inqmlmodule org.kde.kcmutils
   \brief SettingHighlighter automatically impacts the
   representation of an item based on the value of a setting.

   When you are using this item you need to manually manage
   whether the highlighting is enabled or not. For a higher level component
   see SettingStateBinding which will manage the state of the Item.
   \since 6.0
 */
Loader {
    id: root

    active: typeof kcm !== "undefined" && root.target !== null && kcm.defaultsIndicatorsVisible

    /*!
       \brief The graphical element whose appearance will be altered.

       If target is not set, it will try to find the visual parent item.
     */
    property Item target: root.parent

    /*!
       \brief Whether the target will be highlighted.
     */
    property bool highlight: false

    sourceComponent: Item {
        id: helper
        property Item target: root.target.indicator ?? root.target
        property Kirigami.Theme theme: target.Kirigami.Theme
        Binding {
            when: root.highlight
            helper.theme.inherit: false
        }
        Binding {
            when: root.highlight
            helper.theme.focusColor: theme.neutralTextColor
        }
        Binding {
            when: root.highlight
            helper.theme.hoverColor: theme.neutralTextColor
        }
        Binding {
            when: root.highlight
            helper.theme.highlightColor: theme.neutralTextColor
        }
    }
}
