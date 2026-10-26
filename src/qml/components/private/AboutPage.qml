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

    showLibraries: false
    aboutData: {
        "displayName" : metaData.name ?? "",
        "productName" : "",
        "componentName" : "",
        "programLogo": metaData.iconName,
        "shortDescription" : metaData.description ?? "",
        "homepage" : metaData.website ?? "",
        "bugAddress" : metaData.bugReportUrl ?? "",
        "version" : metaData.version ?? "",
        "otherText" : "",
        "authors" : metaData.authors,
        "credits" : metaData.otherContributors,
        "translators" : metaData.translators,
        "licenses" : metaData.license ? [
            {
                "name" : metaData.license,
                "text" : metaData.licenseText,
            }
        ] : [],
        "copyrightStatement" : metaData.copyrightText ?? "",
        "desktopFileName" : ""
    }
}
