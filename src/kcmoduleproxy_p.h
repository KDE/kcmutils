/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCMUTILS_KCMODULEPROXY_P_H
#define KCMUTILS_KCMODULEPROXY_P_H

#include "kcmoduleinfo.h"
#include "kcmoduleproxy.h"
#include <KPluginMetaData>
#include <QLabel>
class QVBoxLayout;

#include <optional>

class KCModuleProxyPrivate
{
    Q_DECLARE_PUBLIC(KCModuleProxy)
protected:
    KCModuleProxyPrivate(KCModuleProxy *_parent,
#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 88)
                         const KCModuleInfo &info,
#endif
                         const QStringList &_args)
        : args(_args)
#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 88)
        , modInfo(info)
#endif
        , parent(_parent)
        , q_ptr(_parent)

    {
    }

    KCModuleProxyPrivate(KCModuleProxy *_parent, const KPluginMetaData &metaData, const QStringList &_args)
        : args(_args)
        , parent(_parent)
        , q_ptr(_parent)
        , metaData(metaData)

    {
    }

    ~KCModuleProxyPrivate()
    {
        delete rootInfo; // Delete before embedWidget!
        delete kcm;
    }

    void loadModule();

    /**
     * Makes sure the proper variables is set and signals are emitted.
     */
    void _k_moduleChanged(bool);

    /**
     * Makes sure the proper variables is set and signals are emitted.
     */
    void _k_moduleDefaulted(bool);

    /**
     * Zeroes d->kcm
     */
    void _k_moduleDestroyed();

    /**
     * Gets called by DCOP when an application closes.
     * Is used to (try to) reload a KCM which previously
     * was loaded.
     */
    void _k_ownerChanged(const QString &service, const QString &oldOwner, const QString &newOwner);

    QStringList args;
    KCModule *kcm = nullptr;
    QVBoxLayout *topLayout = nullptr; /* Contains QScrollView view, and root stuff */
    QLabel *rootInfo = nullptr;
    QString dbusService;
    QString dbusPath;
#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 88)
    KCModuleInfo modInfo;
#endif
    bool changed = false;
    bool defaulted = false;
    KCModuleProxy *parent = nullptr;
    KCModuleProxy *q_ptr = nullptr;
    std::optional<KPluginMetaData> metaData;
};

#endif // KCMUTILS_KCMODULEPROXY_P_H
