/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import "private" as Private

/*!
   \qmltype GridView
   \inqmlmodule org.kde.kcmutils
   \brief A ScrollView containing a GridView, with the default behavior about
   sizing and background as recommended by the user interface guidelines.

   For most KControl modules, it's recommended to use GridViewKCM
   as the root element of your module instead.
   \sa GridViewKCM
 */
QQC2.ScrollView {
    id: scroll

    /*!
       \qmlproperty GridView GridView::view
       \brief Exposes the internal QtQuick.GridView.

       In order to set a model or a delegate to it,
       use the following code:
       \code
       import org.kde.kcmutils as KCMUtils

       KCMUtils.GridView {
           view.model: kcm.model
           view.delegate: KCMUtils.GridDelegate { }
       }
       \endcode
     */
    property alias view: view

    /*!
     */
    property bool framedView: true

    activeFocusOnTab: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    Component.onCompleted: {
        if (background) {
            background.visible = Qt.binding(() => framedView);
        }
    }

    Private.GridViewInternal {
        id: view
    }
    QQC2.ScrollBar.horizontal.visible: false
}
