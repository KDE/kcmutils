/*
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kquickconfigmoduleloader.h"

#include "kcmutils_debug.h"

#include <KPluginFactory>
#include <QCoreApplication>
#include <QJsonArray>
#include <QQmlEngine>

#include "kquickconfigmodule.h"

std::weak_ptr<QQmlEngine> s_kcmutilsCreatedEngine;

KPluginFactory::Result<KQuickConfigModule>
KQuickConfigModuleLoader::loadModule(const KPluginMetaData &metaData, QObject *parent, const QVariantList &args, const std::shared_ptr<QQmlEngine> &engineArg)
{
    const auto factoryResult = KPluginFactory::loadFactory(metaData);
    KPluginFactory::Result<KQuickConfigModule> result;
    if (!factoryResult) {
        result.errorReason = factoryResult.errorReason;
        result.errorString = factoryResult.errorString;
        result.errorText = factoryResult.errorText;
        return result;
    }
    KPluginFactory *factory = factoryResult.plugin;

    factory->setMetaData(KPluginMetaData(metaData));

    const QVariantList pluginArgs = QVariantList(args) << metaData.rawData().value(QLatin1String("X-KDE-KCM-Args")).toArray().toVariantList();
    if (const auto kcm = factory->create<KQuickConfigModule>(parent, pluginArgs)) {
        auto engine = engineArg;
        if (!engine) {
            auto applicationEngine = QCoreApplication::instance()->property("__qmlEngine").value<std::weak_ptr<QQmlEngine>>();
            if (!applicationEngine.expired()) {
                engine = applicationEngine.lock();
            }
        }

        if (!engine && !s_kcmutilsCreatedEngine.expired()) {
            engine = s_kcmutilsCreatedEngine.lock();
        }

        if (!engine) {
            qWarning() << "Could not find a shared engine to use to load KQuickConfigModule" << metaData.pluginId();
            engine = std::make_shared<QQmlEngine>();

            if (s_kcmutilsCreatedEngine.expired()) {
                s_kcmutilsCreatedEngine = engine;
            }
        }

        kcm->setInternalEngine(engine);

        result.plugin = kcm;
        qCDebug(KCMUTILS_LOG) << "loaded QML KCM" << metaData.fileName();
    } else {
        result.errorReason = KPluginFactory::INVALID_KPLUGINFACTORY_INSTANTIATION;
    }

    return result;
}
