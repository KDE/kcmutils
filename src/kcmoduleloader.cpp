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
#include "kcmoduleloaderqml.h"
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
        QString realDetails = details;
        if (realDetails.isNull()) {
            realDetails = i18n(
                "<qt><p>Possible reasons:<ul><li>An error occurred during your last "
                "system upgrade, leaving an orphaned control module behind</li><li>You have old third party "
                "modules lying around.</li></ul></p><p>Check these points carefully and try to remove "
                "the module mentioned in the error message. If this fails, consider contacting "
                "your distributor or packager.</p></qt>");
        }

        QVBoxLayout *topLayout = new QVBoxLayout(widget());
        QLabel *lab = new QLabel(msg, widget());
        lab->setWordWrap(true);
        topLayout->addWidget(lab);
        lab = new QLabel(realDetails, widget());
        lab->setWordWrap(true);
        topLayout->addWidget(lab);
    }
};

KCModule *KCModuleLoader::loadModule(const KPluginMetaData &metaData, QWidget *parent, const QVariantList &args, const std::shared_ptr<QQmlEngine> &eng)
{
    if (!KAuthorized::authorizeControlModule(metaData.pluginId())) {
        return new KCMError(i18n("The module %1 is disabled.", metaData.pluginId()), i18n("The module has been disabled by the system administrator."), parent);
    }

    const auto qmlKcm = KCModuleLoaderQml::loadModule(metaData, parent, args, eng).plugin;
    if (qmlKcm) {
        if (!qmlKcm->mainUi()) {
            return new KCMError(i18n("Error loading QML file."), qmlKcm->errorString(), parent);
        }
        qCDebug(KCMUTILS_LOG) << "loaded KCM" << metaData.fileName();
        return new KCModuleQml(qmlKcm, parent, args);
    }

    const auto kcmoduleResult =
        KPluginFactory::instantiatePlugin<KCModule>(metaData,
                                                    parent,
                                                    QVariantList(args) << metaData.rawData().value(QStringLiteral("X-KDE-KCM-Args")).toArray());

    if (kcmoduleResult) {
        qCDebug(KCMUTILS_LOG) << "loaded KCM" << metaData.fileName();
        return kcmoduleResult.plugin;
    }

    return new KCMError(QString(), kcmoduleResult.errorString, parent);
}
