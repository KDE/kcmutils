/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "../core/kpluginproxymodel.h"

class KCMUtilsQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void initializeEngine(QQmlEngine * /*engine*/, const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.kcmutils"));
        qmlRegisterType<KPluginProxyModel>("org.kde.kcmutils.private", 1, 0, "ProxyModel");
    }

    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.kcmutils"));
        qmlRegisterModule(uri, 1, 0);
    };
};

#include "qmlplugin.moc"
