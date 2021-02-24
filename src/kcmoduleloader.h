/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Daniel Molkentin <molkentin@kde.org>
    SPDX-FileCopyrightText: 2006 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCMODULELOADER_H
#define KCMODULELOADER_H

#include <KCModule>
#include <kcmoduleinfo.h>

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
 * Loads a @ref KCModule. If loading fails a zero pointer is returned.
 * @param module what module to load
 * @param report see ErrorReporting
 *
 * @return a pointer to the loaded @ref KCModule
 */
KCMUTILS_EXPORT KCModule *loadModule(const KCModuleInfo &module, ErrorReporting report, QWidget *parent = nullptr, const QStringList &args = QStringList());

/**
 * Loads a @ref KCModule. If loading fails a zero pointer is returned.
 *
 * @param module what module to load
 * @param report see ErrorReporting
 *
 * @return a pointer to the loaded @ref KCModule
 */
KCMUTILS_EXPORT KCModule *loadModule(const QString &module, ErrorReporting report, QWidget *parent = nullptr, const QStringList &args = QStringList());

/**
 * Unloads the module's library
 * @param mod What module to unload for
 */
KCMUTILS_EXPORT void unloadModule(const KCModuleInfo &mod);

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

/**
 * For a specified module, return false if configuration is not the default one and true in all other case including if module is not found
 * @param module what module to load
 *
 * @return a boolean representing if module configuration is in default state
 *
 * @since 5.74
 */
KCMUTILS_EXPORT bool isDefaults(const KCModuleInfo &module, const QStringList &args = QStringList());
}

#endif // KCMODULELOADER_H
