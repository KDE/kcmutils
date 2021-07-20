/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
    SPDX-FileCopyrightText: 2003 Daniel Molkentin <molkentin@kde.org>
    SPDX-FileCopyrightText: 2003, 2006 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCMODULEINFO_H
#define KCMODULEINFO_H

#include <KService>
#include <kcmutils_export.h>

class KPluginInfo;
class QString;
class QStringList;

/**
 * A class that provides information about a KCModule
 *
 * KCModuleInfo provides various technical information, such as icon, library
 * etc. about a KCModule.n
 * @note Any values set with the set* functions is not
 * written back with KCModuleInfo it only reads value from the desktop file.
 *
 * @internal
 * @author Matthias Hoelzer-Kluepfel <mhk@kde.org>
 * @author Matthias Elter <elter@kde.org>
 * @author Daniel Molkentin <molkentin@kde.org>
 *
 */
class KCMUTILS_EXPORT KCModuleInfo // krazy:exclude=dpointer (implicitly shared)
{
public:
    /**
     * Constructs a KCModuleInfo.
     * @note a KCModuleInfo object will have to be manually deleted, it is not
     * done automatically for you.
     * @param desktopFile the desktop file representing the module, or
     * the name of the module.
     */
    KCModuleInfo(const QString &desktopFile);

    /**
     * Same as above but takes a KPluginInfo as argument.
     * This allows to encapsulate both the case of KService (desktop file)
     * and the case of KPluginMetaData (JSon data in .so file) under the same API.
     *
     * @param pluginInfo specifies the module
     * @since 5.70
     */
    KCModuleInfo(const KPluginInfo &pluginInfo);

    /**
     * Same as above but takes a KService::Ptr as argument.
     *
     * @note @p moduleInfo must be a valid pointer.
     *
     * @param moduleInfo specifies the module
     */
    KCModuleInfo(KService::Ptr moduleInfo);

    /**
     * Copy constructor
     * @param rhs specifies the module info to copy
     */
    KCModuleInfo(const KCModuleInfo &rhs);

    /**
     * Same as above but creates an empty KCModuleInfo.
     * You should not normally call this.
     */
    KCModuleInfo();

    /**
     * Assignment operator
     */
    KCModuleInfo &operator=(const KCModuleInfo &rhs);

    /**
     * Returns true if @p rhs describes the same KCModule as this object.
     */
    bool operator==(const KCModuleInfo &rhs) const;

    /**
     * @return true if @p rhs is not equal itself
     */
    bool operator!=(const KCModuleInfo &rhs) const;

    /**
     * Default destructor.
     */
    ~KCModuleInfo();

    /**
     * Returns true if the KCM was found
     * @since 5.71
     */
    bool isValid() const;

    /**
     * @return the filename of the .desktop file that describes the KCM
     */
    QString fileName() const;

    /**
     * @return the keywords associated with this KCM.
     */
    QStringList keywords() const;

    /**
     * @return the module\'s (translated) name
     */
    QString moduleName() const;

    /**
     * @return a QExplicitlySharedDataPointer to KService created from the modules .desktop file
     * @warning This will be null if this KCModuleInfo was created from a KPluginInfo coming from KPluginMetaData.
     * Prefer using pluginInfo() instead, which works for both kinds.
     */
    KService::Ptr service() const; // TODO KF6 REMOVE

    /**
     * @return the KPluginInfo containing more information about this module
     * @since 5.70
     */
    KPluginInfo pluginInfo() const;

    /**
     * @return the module's (translated) comment field
     */
    QString comment() const;

    /**
     * @return the module's icon name
     */
    QString icon() const;

    /**
     * @return the path of the module's documentation
     */
    QString docPath() const;

    /**
     * @return the library name
     */
    QString library() const;

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 85)
    /**
     * @return a handle (the contents of the X-KDE-FactoryName field if it exists,
     * else the same as the library name)
     * @deprecated Since 5.85, use KPluginMetaData::pluginId or KCModuleInfo::library instead
     */
    KCMUTILS_DEPRECATED_VERSION(5, 85, "Use KPluginMetaData::pluginId or KCModuleInfo::library instead")
    QString handle() const;
#endif

    /**
     * @return the weight of the module which determines the order of the pages in
     * the KCMultiDialog. It's set by the X-KDE-Weight field.
     */
    int weight() const;

    /**
     * @return The value associated to the @p key. You can use it if you
     *         want to read custom values. To do this you need to define
     *         your own servicetype and add it to the ServiceTypes keys.
     * @since 5.71
     */
    QVariant property(const QString &key) const;

private:
    class Private;
    Private *d;
};

#endif // KCMODULEINFO_H
