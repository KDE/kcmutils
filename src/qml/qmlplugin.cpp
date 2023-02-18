/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QQmlExtensionPlugin>

#include "../core/kpluginproxymodel.h"
#include "kquickconfigmodule.h"

class KCMUtilsQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.kcmutils"));
        qmlRegisterType<KPluginProxyModel>("org.kde.kcmutils.private", 1, 0, "ProxyModel");
        qmlRegisterModule(uri, 1, 0);
        qmlRegisterUncreatableType<KQuickConfigModule>("org.kde.kcmutils", 1, 0, "ConfigModule", QLatin1String("Do not create objects of type ConfigModule"));
    };
};

#include "qmlplugin.moc"
