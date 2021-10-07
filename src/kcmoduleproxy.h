/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCMUTILS_KCMODULEPROXY_H
#define KCMUTILS_KCMODULEPROXY_H

#include <QStringList>

#include <KCModule>
#include <KService>
#include <kcmutils_export.h>

class KAboutData;
class KCModuleInfo;
class KCModuleProxyPrivate;
class KPluginMetaData;

/**
 *
 * @brief Encapsulates a KCModule for embedding.
 *
 * KCModuleProxy is a wrapper for KCModule intended for cases where
 * modules are to be displayed. It ensures layout is consistent
 * and in general takes care of the details
 * needed for making a module available in an interface. A KCModuleProxy
 * can be treated as a QWidget, without worrying about the details specific
 * for modules such as library loading. KCModuleProxy is not a sub class of KCModule
 * but its API closely resembles KCModule's.\n
 * Usually, an instance is created by passing one of the constructors a KService::Ptr,
 * KCModuleInfo or simply the name of the module and then added to the layout as any
 * other widget. \n
 * When the user has changed the module, changed(bool) as well as changed(KCModuleProxy *)
 * is emitted. KCModuleProxy does not take care of prompting for saving - if the object is deleted while
 * changes is not saved the changes will be lost. changed() returns true if changes are unsaved. \n
 * \n
 * KCModuleProxy does not take care of authorization of KCModules. \n
 * KCModuleProxy implements lazy loading, meaning the library will not be loaded or
 * any other initialization done before its show() function is called. This means
 * modules will only be loaded when they are actually needed as well as it is possible to
 * load many KCModuleProxy without any speed penalty.
 *
 * KCModuleProxy should be used in all cases where modules are embedded in order to
 * promote code efficiency and usability consistency.
 *
 * @author Frans Englich <frans.englich@telia.com>
 * @author Matthias Kretz <kretz@kde.org>
 *
 */
class KCMUTILS_EXPORT KCModuleProxy : public QWidget
{
    Q_DECLARE_PRIVATE(KCModuleProxy)
    Q_OBJECT
public:
#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 88)
    /**
     * Constructs a KCModuleProxy from a KCModuleInfo class.
     *
     * @param info The KCModuleInfo to construct the module from.
     * @param parent the parent QWidget.
     * @param args This is used in the implementation and is internal.
     * Use the default.
     * @deprecated Since 5.88, use KCModuleProxy(KPluginMetaData, QWidget *, QStringList) instead
     */
    KCMUTILS_DEPRECATED_VERSION(5, 88, "Use KCModuleProxy(KPluginMetaData, QWidget *, QStringList) instead")
    explicit KCModuleProxy(const KCModuleInfo &info, QWidget *parent = nullptr, const QStringList &args = QStringList());
#endif

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 88)
    /**
     * Constructs a KCModuleProxy from a module's service name, which is
     * equivalent to the desktop file for the kcm without the ".desktop" part.
     * Otherwise equal to the one above.
     *
     * @param serviceName The module's service name to construct from.
     * @param parent the parent QWidget.
     * @param args This is used in the implementation and is internal.
     * Use the default.
     * @deprecated Since 5.88, use KCModuleProxy(KPluginMetaData, QWidget *, QStringList) instead
     */
    KCMUTILS_DEPRECATED_VERSION(5, 88, "Use KCModuleProxy(KPluginMetaData, QWidget *, QStringList) instead")
    explicit KCModuleProxy(const QString &serviceName, QWidget *parent = nullptr, const QStringList &args = QStringList());
#endif

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 88)
    /**
     * Constructs a KCModuleProxy from KService. Otherwise equal to the one above.
     *
     * @param service The KService to construct from.
     * @param parent the parent QWidget.
     * @param args This is used in the implementation and is internal.
     * Use the default.
     * @deprecated Since 5.88, use KCModuleProxy(KPluginMetaData, QWidget *, QStringList) instead
     */
    KCMUTILS_DEPRECATED_VERSION(5, 88, "Use KCModuleProxy(KPluginMetaData, QWidget *, QStringList) instead")
    explicit KCModuleProxy(const KService::Ptr &service, QWidget *parent = nullptr, const QStringList &args = QStringList());
#endif

    /**
     * Constructs a KCModuleProxy from KPluginMetaData
     * @since 5.84
     */
    explicit KCModuleProxy(const KPluginMetaData &metaData, QWidget *parent = nullptr, const QStringList &args = QStringList());

    /**
     * Default destructor
     */
    ~KCModuleProxy() override;

    /**
     * Calling it will cause the contained module to
     * run its load() routine.
     */
    void load();

    /**
     * Calling it will cause the contained module to
     * run its save() routine.
     *
     * If the module was not modified, it will not be asked
     * to save.
     */
    void save();

    /**
     * @return the module's quickHelp();
     */
    QString quickHelp() const;

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 85)
    /**
     * @return the module's aboutData()
     * @deprecated since 5.85, use metaData() instead.
     */
    KCMUTILS_DEPRECATED_VERSION(5, 85, "Use metaData() instead")
    const KAboutData *aboutData() const;
#endif

    /**
     * @return what buttons the module
     * needs
     */
    KCModule::Buttons buttons() const;

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 87)
    /**
     * @return true if the module is modified
     * and needs to be saved.
     * @deprecated Since 5.87, use isChanged instead
     */
    KCMUTILS_DEPRECATED_VERSION(5, 87, "use isChanged instead")
    bool changed() const;
#endif
    /**
     * @return true if the module is modified
     * and needs to be saved.
     * @since 5.87
     */
    bool isChanged() const;

    /**
     * @return true if the module is matching default settings
     *
     * @since 5.65
     */
    bool defaulted() const;

    /**
     * Access to the actual module.
     * It may return NULL if anything goes wrong.
     *
     * @return the encapsulated module.
     */
    KCModule *realModule() const;

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 88)
    /**
     * @return a KCModuleInfo for the encapsulated
     * module
     * @deprecated Since 5.87 method is obsolete with deprecation of KCModuleInfo constructor, use metaData() instead
     */
    KCMUTILS_DEPRECATED_VERSION(5, 88, "method is obsolete with deprecation of KCModuleInfo constructor, use metaData() instead")
    KCModuleInfo moduleInfo() const;
#endif

    /**
     * Returns the KPluginMetaData used to load the KCM. If the KCM is not loaded using KPluginMetaData the returned object is invalid.
     * @return a KPluginMetaData for the encapsulated module
     * @since 5.84
     */
    KPluginMetaData metaData() const;

    /**
     * Returns the D-Bus Service name
     */
    QString dbusService() const;
    /**
     * Returns the D-Bus Path
     */
    QString dbusPath() const;
    /**
     * Returns the recommended minimum size for the widget
     */
    QSize minimumSizeHint() const override;

    /**
     * Show or hide an indicator when settings have changed from their default value
     *
     * @since 5.73
     */
    void setDefaultsIndicatorsVisible(bool show);

public Q_SLOTS:

    /**
     * Calling it will cause the contained module to
     * load its default values.
     */
    void defaults();

    /**
     * Calling this, results in deleting the contained
     * module, and unregistering from DCOP. A similar result is achieved
     * by deleting the KCModuleProxy itself.
     */
    void deleteClient();

Q_SIGNALS:

    /*
     * This signal is emitted when the contained module is changed.
     */
    void changed(bool state);

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 87)
    /**
     * This is emitted in the same situations as in the one above. Practical
     * when several KCModuleProxys are loaded.
     * @deprecated Since 5.87, use changed(bool) instead
     */
    KCMUTILS_DEPRECATED_VERSION(5, 87, "use changed(bool) instead")
    void changed(KCModuleProxy *mod);
#endif

    /**
     * When a module running with root privileges and exits, returns to normal mode, the
     * childClosed() signal is emitted.
     */
    void childClosed();

    /*
     * This signal is relayed from the encapsulated module, and
     * is equivalent to the module's own quickHelpChanged() signal.
     */
    void quickHelpChanged();

protected:
    /**
     * Reimplemented for internal purposes. Makes sure the encapsulated
     * module is loaded before the show event is taken care of.
     */
    void showEvent(QShowEvent *) override;

protected:
    KCModuleProxyPrivate *const d_ptr;

private:
    Q_PRIVATE_SLOT(d_func(), void _k_moduleChanged(bool))
    Q_PRIVATE_SLOT(d_func(), void _k_moduleDefaulted(bool))
    Q_PRIVATE_SLOT(d_func(), void _k_moduleDestroyed())
    Q_PRIVATE_SLOT(d_func(), void _k_ownerChanged(const QString &service, const QString &oldOwner, const QString &newOwner))
};

#endif // KUTILS_KCMODULEPROXY_H
