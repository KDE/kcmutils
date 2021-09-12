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
#include <kcmoduleinfo.h>
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

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 88)
/**
 * Loads a @ref KCModule. If loading fails a zero pointer is returned.
 * @param module what module to load
 * @param report see ErrorReporting
 *
 * @return a pointer to the loaded @ref KCModule
 * @deprecated Since 5.88, use loadModule(KPluginMetaData, QWidget *, QVariantList) instead
 */
KCMUTILS_DEPRECATED_VERSION(5, 88, "use loadModule(KPluginMetaData, QWidget *, QVariantList) instead")
KCMUTILS_EXPORT KCModule *loadModule(const KCModuleInfo &module, ErrorReporting report, QWidget *parent = nullptr, const QStringList &args = QStringList());
#endif

/**
 * Loads a @ref KCModule. If loading fails a KCM which displays an error message is returned.

 * @param metaData KPluginMetaData for loading the plugin
 * @return a pointer to the loaded @ref KCModule
 * @since 5.84
 */
KCMUTILS_EXPORT KCModule *loadModule(const KPluginMetaData &metaData, QWidget *parent = nullptr, const QVariantList &args = {});

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 88)
/**
 * Loads a @ref KCModule. If loading fails a zero pointer is returned.
 *
 * @param module what module to load
 * @param report see ErrorReporting
 *
 * @return a pointer to the loaded @ref KCModule
 * @deprecated Since 5.88, use loadModule(KPluginMetaData, QWidget *, QVariantList) instead
 */
KCMUTILS_DEPRECATED_VERSION(5, 88, "use loadModule(KPluginMetaData, QWidget *, QVariantList) instead")
KCMUTILS_EXPORT KCModule *loadModule(const QString &module, ErrorReporting report, QWidget *parent = nullptr, const QStringList &args = QStringList());
#endif

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 88)
/**
 * Unloads the module's library
 * @param mod What module to unload for
 * @deprecated Since 5.88, use QPluginLoader directly
 */
KCMUTILS_DEPRECATED_VERSION(5, 88, "use QPluginLoader directly")
KCMUTILS_EXPORT void unloadModule(const KCModuleInfo &mod);
#endif

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

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 88)
/**
 * For a specified module, return false if configuration is not the default one and true in all other case including if module is not found
 * @param module what module to load
 *
 * @return a boolean representing if module configuration is in default state
 *
 * @since 5.74
 * @deprecated Since 5.88, use KPluginFactory::instantiatePlugin<KCModuleData> and save the result in a QScopedPointer instead
 */
KCMUTILS_DEPRECATED_VERSION(5, 88, "use KPluginFactory::instantiatePlugin<KCModuleData> and save the result in a QScopedPointer instead")
KCMUTILS_EXPORT bool isDefaults(const KCModuleInfo &module, const QStringList &args = QStringList());
#endif

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 88)
/**
 * Loads a @ref KCModuleData. If fails to load, return nullptr.
 *
 * @param module what module to load
 *
 * @since 5.81
 * @deprecated Since 5.88, use KPluginFactory::instantiatePlugin<KCModuleData> instead
 */
KCMUTILS_DEPRECATED_VERSION(5, 88, "use KPluginFactory::instantiatePlugin<KCModuleData> instead")
KCMUTILS_EXPORT KCModuleData *loadModuleData(const KCModuleInfo &module, const QStringList &args = QStringList());
#endif
}

#endif // KCMODULELOADER_H
