/*
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kquickconfigmoduleloader.h"

#include "kcmutils_debug.h"

#include <KPluginFactory>
#include <QJsonArray>
#include <QQmlEngine>

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

    const QVariantList args2 = QVariantList(args) << metaData.rawData().value(QStringLiteral("X-KDE-KCM-Args")).toArray();
    factory->setMetaData(KPluginMetaData(metaData));

    const auto kcm = factory->create<KQuickConfigModule>(parent, args2);
    if (kcm) {
        const std::shared_ptr<QQmlEngine> engine =
            engineArg ? engineArg : (s_kcmutilsCreatedEngine.expired() ? std::make_shared<QQmlEngine>() : s_kcmutilsCreatedEngine.lock());

        if (!engineArg && s_kcmutilsCreatedEngine.expired()) {
            s_kcmutilsCreatedEngine = engine;
        }
        kcm->setInternalEngine(engine);

        result.plugin = kcm;
        qCDebug(KCMUTILS_LOG) << "loaded QML KCM" << metaData.fileName();
    } else {
        result.errorReason = KPluginFactory::INVALID_KPLUGINFACTORY_INSTANTIATION;
    }

    return result;
}
