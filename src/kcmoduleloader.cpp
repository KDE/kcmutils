/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
    SPDX-FileCopyrightText: 2003, 2004, 2006 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcmoduleloader.h"
#include "kcmoduledata.h"
#include "kcmoduleqml_p.h"
#include <kcmutils_debug.h>

#include <QJsonArray>
#include <QLabel>
#include <QLibrary>
#include <QVBoxLayout>

#include <KAboutData>
#include <KAuthorized>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <memory>
#include <qqmlengine.h>

#include "qml/configmodule.h"

using namespace KCModuleLoader;

/***************************************************************/
/**
 * When something goes wrong in loading the module, this one
 * jumps in as a "dummy" module.
 */
class KCMError : public KCModule
{
public:
    KCMError(const QString &msg, const QString &details, QWidget *parent)
        : KCModule(parent)
    {
        QVBoxLayout *topLayout = new QVBoxLayout(widget());
        QLabel *lab = new QLabel(msg, widget());
        lab->setWordWrap(true);
        topLayout->addWidget(lab);
        lab = new QLabel(details, widget());
        lab->setWordWrap(true);
        topLayout->addWidget(lab);
    }
};

KCModule *KCModuleLoader::loadModule(const KPluginMetaData &metaData, QWidget *parent, const QVariantList &args, const std::shared_ptr<QQmlEngine> &eng)
{
    static std::weak_ptr<QQmlEngine> createdEngine;
    std::shared_ptr<QQmlEngine> engine = eng ? eng : (createdEngine.expired() ? std::make_shared<QQmlEngine>() : createdEngine.lock());
    if (!eng && createdEngine.expired()) {
        createdEngine = engine;
    }

    if (!KAuthorized::authorizeControlModule(metaData.pluginId())) {
        return reportError(ErrorReporting::Inline,
                           i18n("The module %1 is disabled.", metaData.pluginId()),
                           i18n("The module has been disabled by the system administrator."),
                           parent);
    }
    const QVariantList args2 = QVariantList(args) << metaData.rawData().value(QStringLiteral("X-KDE-KCM-Args")).toArray() << QVariant::fromValue(engine);

    auto factoryResult = KPluginFactory::loadFactory(metaData);
    if (!factoryResult) {
        // This is where QML KCMs used to be before the namespaces were changed based on https://phabricator.kde.org/T14517
        // But the X-KDE-Library did not reflect this change, instead the "kcms" namespace was prepended
        if (KPluginMetaData data(QLatin1String("kcms/") + metaData.fileName()); data.isValid()) {
            factoryResult = KPluginFactory::loadFactory(data);
        }
    }

    if (!factoryResult) {
        return reportError(ErrorReporting::Inline, factoryResult.errorString, QString(), parent);
    }

    KPluginFactory *factory = factoryResult.plugin;

    const auto qmlKCMResult = factory->create<KQuickAddons::ConfigModule>(parent, args2);

    if (qmlKCMResult) {
        std::unique_ptr<KQuickAddons::ConfigModule> kcm(qmlKCMResult);

        if (!kcm->mainUi()) {
            return reportError(ErrorReporting::Inline, i18n("Error loading QML file."), kcm->errorString(), parent);
        }
        qCDebug(KCMUTILS_LOG) << "loaded KCM" << factory->metaData().pluginId() << "from path" << factory->metaData().fileName();
        return new KCModuleQml(engine, std::move(kcm), parent, args2);
    }

    const auto kcmoduleResult = factory->create<KCModule>(parent, args2);

    if (kcmoduleResult) {
        qCDebug(KCMUTILS_LOG) << "loaded KCM" << factory->metaData().pluginId() << "from path" << factory->metaData().fileName();
        return kcmoduleResult;
    }

    return reportError(ErrorReporting::Inline, QString(), QString(), parent);
}

KCModule *KCModuleLoader::reportError(ErrorReporting report, const QString &text, const QString &details, QWidget *parent)
{
    QString realDetails = details;
    if (realDetails.isNull()) {
        realDetails = i18n(
            "<qt><p>Possible reasons:<ul><li>An error occurred during your last "
            "system upgrade, leaving an orphaned control module behind</li><li>You have old third party "
            "modules lying around.</li></ul></p><p>Check these points carefully and try to remove "
            "the module mentioned in the error message. If this fails, consider contacting "
            "your distributor or packager.</p></qt>");
    }
    if (report & KCModuleLoader::Dialog) {
        KMessageBox::detailedError(parent, text, realDetails);
    }
    if (report & KCModuleLoader::Inline) {
        return new KCMError(text, realDetails, parent);
    }
    return nullptr;
}

