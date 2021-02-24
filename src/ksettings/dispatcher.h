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
 */
KCMUTILS_EXPORT void registerComponent(const QString &componentName, QObject *recv, const char *slot);

/**
 * @return the KConfig object that belongs to the componentName
 */
KCMUTILS_EXPORT KSharedConfig::Ptr configForComponentName(const QString &componentName);

/**
 * @return a list of all the componentData names that are currently
 * registered
 */
KCMUTILS_EXPORT QList<QString> componentNames();

/**
 * Call this function when the configuration belonging to the associated
 * componentData name has changed. The registered slot will be called.
 *
 * @param componentName The value of X-KDE-ParentComponents.
 */
KCMUTILS_EXPORT void reparseConfiguration(const QString &componentName);

/**
 * When this function is called the KConfig objects of all the registered
 * instances are sync()ed. This is useful when some other KConfig
 * objects will read/write from/to the same config file, so that you
 * can first write out the current state of the KConfig objects.
 */
KCMUTILS_EXPORT void syncConfiguration();
} // namespace Dispatcher

}
#endif // KSETTINGS_DISPATCHER_H
