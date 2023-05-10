/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QQmlExtensionPlugin>

#include "../core/kpluginproxymodel.h"
#include "kcmlauncher_p.h"
#include "kquickconfigmodule.h"
#include "settinghighlighterprivate.h"
#include "settingstateproxy.h"

class KCMUtilsQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void registerTypes(const char *uri) override
    {
        qmlRegisterType<KPluginProxyModel>("org.kde.kcmutils.private", 1, 0, "ProxyModel");
        qmlRegisterModule(uri, 1, 0);
        qmlRegisterUncreatableType<KQuickConfigModule>(uri, 1, 0, "ConfigModule", QLatin1String("Do not create objects of type ConfigModule"));
        qmlRegisterType<SettingStateProxy>(uri, 1, 0, "SettingStateProxy");
        qmlRegisterType<SettingHighlighterPrivate>("org.kde.kcmutils.private", 1, 0, "SettingHighlighterPrivate");
        qmlRegisterSingletonType<KCMLauncher>(uri, 1, 0, "KCMLauncher", [](QQmlEngine *, QJSEngine *) {
            return new KCMLauncher();
        });
    }
};

#include "qmlplugin.moc"
