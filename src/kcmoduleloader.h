/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Daniel Molkentin <molkentin@kde.org>
    SPDX-FileCopyrightText: 2006 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCMODULELOADER_H
#define KCMODULELOADER_H

#include <KCModule>
#include <KPluginMetaData>

#include <kcmoduledata.h>

class QWidget;

/**
 * @short Loads a KControl Module.
 *
 * KCModuleLoader tries in several ways
 * to locate and load a KCModule. If loading fails a
 * zero pointer is returned. \n
 * It is very unlikely KCModuleLoader is what you want
 * and @ref KCModuleProxy suits your needs.
 *
 * @author Matthias Hoelzer-Kluepfel <mhk@kde.org>
 * @author Frans Englich <frans.englich@telia.com>
 * @internal
 **/
namespace KCModuleLoader
{
/**
 * Determines the way errors are reported
 */
enum ErrorReporting {
    /**
     * no error reporting is done
     * */
    None = 0,
    /**
     * the error report is shown instead of the
     * KCModule that should have * been loaded
     */
    Inline = 1,
    /**
     * shows a dialog with the error report
     */
    Dialog = 2,
    /**
     * does both Inline and Dialog
     */
    Both = 3,
};

/**
 * Loads a @ref KCModule. If loading fails a KCM which displays an error message is returned.
 * Starting from 5.91, the validity of the @p metaData parameter does not need to be checked when calling this function.
 *
 * @param metaData KPluginMetaData for loading the plugin
 * @return a pointer to the loaded @ref KCModule
 * @since 5.84
 */
KCMUTILS_EXPORT KCModule *loadModule(const KPluginMetaData &metaData, QWidget *parent = nullptr, const QVariantList &args = {});

/**
 * Returns a KCModule containing the messages @p report and @p text.
 *
 * @param report the type of error reporting, see ErrorReporting
 * @param text the main message
 * @param details any additional details
 *
 * @internal
 */
KCMUTILS_EXPORT KCModule *reportError(ErrorReporting report, const QString &text, const QString &details, QWidget *parent);

}

#endif // KCMODULELOADER_H
