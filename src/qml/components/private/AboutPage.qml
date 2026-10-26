/*
    SPDX-FileCopyrightText: 2026 Sam Crawford <samlkcrawford@pm.me>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.kirigami as Kirigami

/*!
 * \internal
 * About page for plugins, pushed onto the KCM page stack.
 */
Kirigami.AboutPage {
    required property var metaData

    aboutData: {
        "displayName" : metaData.name ?? "",
        "productName" : "",
        "componentName" : "",
        "programLogo": metaData.iconName,
        "shortDescription" : metaData.description ?? "",
        "homepage" : metaData.website ?? "",
        "bugAddress" : "",
        "version" : metaData.version ?? "",
        "otherText" : "",
        "authors" : metaData.authors,
        "credits" : metaData.otherContributors,
        "translators" : metaData.translators,
        "licenses" : [
            {
                "name" : metaData.license,
                "text" : metaData.licenseText,
            }
        ],
        "copyrightStatement" : "",
        "desktopFileName" : ""
    }
}
