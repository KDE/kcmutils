/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSETTINGS_DISPATCHER_H
#define KSETTINGS_DISPATCHER_H

#include <KSharedConfig>
#include <QObject>
#include <kcmutils_export.h>

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 85)
namespace KSettings
{
/**
 * @short Dispatch change notifications from the KCMs to the program.
 *
 * Since your program does not have direct control over the KCMs that get loaded
 * into KSettings::Dialog you need a way to get notified. This is what you do:
 * \code
 * Dispatcher::registerComponent(componentData(), this, "loadSettings");
 * \endcode
 *
 * @author Matthias Kretz <kretz@kde.org>
 */
namespace Dispatcher
{
/**
 * Register a slot to be called when the configuration for the componentData
 * has changed. @p componentName is the string that is passed to KPluginFactory (if it is used).
 * You can query it with MyPluginFactory::componentName(), or from a KAboutData.
 * componentName is also the same name that is put into the
 * .desktop file of the KCMs for the X-KDE-ParentComponents.
 *
 * @param componentName     The name of the component
 * @param recv         The object that should receive the signal
 * @param slot         The slot to be called: "slotName"
 * @deprecated Since 5.85, connect to the @ref KCMultiDialog::configCommitted() signal of the dialog instead and check the componentName
 */

KCMUTILS_DEPRECATED_VERSION(5, 85, "Connect to the KCMultiDialog::configCommitted() signal of the dialog instead and check the componentName")
KCMUTILS_EXPORT void registerComponent(const QString &componentName, QObject *recv, const char *slot);

/**
 * @return the KConfig object that belongs to the componentName
 * @deprecated Since 5.85, use KSharedConfig::openConfig(componentName + QStringLiteral("rc")) instaed
 */
KCMUTILS_DEPRECATED_VERSION(5, 85, "Use KSharedConfig::openConfig(componentName + QStringLiteral(\"rc\")) instaed")
KCMUTILS_EXPORT KSharedConfig::Ptr configForComponentName(const QString &componentName);

/**
 * @return a list of all the componentData names that are currently
 * registered
 * @deprecated Since 5.85, this method is obsolete, see @p registerComponent API docs.
 */
KCMUTILS_DEPRECATED_VERSION(5, 85, "method is obsolete, see KSettings::registerComponent API docs")
KCMUTILS_EXPORT QList<QString> componentNames();

/**
 * Call this function when the configuration belonging to the associated
 * componentData name has changed. The registered slot will be called.
 *
 * @param componentName The value of X-KDE-ParentComponents.
 * @deprecated Since 5.85, manually reparse the config instead
 */
KCMUTILS_DEPRECATED_VERSION(5, 85, "Manually reparse the config instead")
KCMUTILS_EXPORT void reparseConfiguration(const QString &componentName);

/**
 * When this function is called the KConfig objects of all the registered
 * instances are sync()ed. This is useful when some other KConfig
 * objects will read/write from/to the same config file, so that you
 * can first write out the current state of the KConfig objects.
 * @deprecated Since 5.85, manually sync the config instead
 */
KCMUTILS_DEPRECATED_VERSION(5, 85, "Manually sync the config instead")
KCMUTILS_EXPORT void syncConfiguration();
} // namespace Dispatcher

}
#endif // KSETTINGS_DISPATCHER_H
#endif
