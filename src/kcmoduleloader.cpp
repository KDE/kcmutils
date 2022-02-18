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

#include <QLabel>
#include <QLibrary>
#include <QVBoxLayout>

#include <KAboutData>
#include <KAuthorized>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginInfo>

#include <KQuickAddons/ConfigModule>

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
        QVBoxLayout *topLayout = new QVBoxLayout(this);
        QLabel *lab = new QLabel(msg, this);
        lab->setWordWrap(true);
        topLayout->addWidget(lab);
        lab = new QLabel(details, this);
        lab->setWordWrap(true);
        topLayout->addWidget(lab);
    }
};

KCModule *KCModuleLoader::loadModule(const KPluginMetaData &metaData, QWidget *parent, const QVariantList &args)
{
    if (!KAuthorized::authorizeControlModule(metaData.pluginId())) {
        return reportError(ErrorReporting::Inline,
                           i18n("The module %1 is disabled.", metaData.pluginId()),
                           i18n("The module has been disabled by the system administrator."),
                           parent);
    }
    const QVariantList args2 = QVariantList(args) << metaData.rawData().value(QStringLiteral("X-KDE-KCM-Args")).toArray();

    auto factoryResult = KPluginFactory::loadFactory(metaData);
    QString pluginKeyword = metaData.value(QStringLiteral("X-KDE-PluginKeyword"));
    if (!factoryResult) {
        // This is where QML KCMs used to be before the namespaces were changed based on https://phabricator.kde.org/T14517
        // But the X-KDE-Library did not reflect this change, instead the "kcms" namespace was prepended
        if (KPluginMetaData data(QLatin1String("kcms/") + metaData.fileName()); data.isValid()) {
            factoryResult = KPluginFactory::loadFactory(data);
            pluginKeyword.clear();
        }
    }

    if (!factoryResult) {
        return reportError(ErrorReporting::Inline, factoryResult.errorString, QString(), parent);
    }

    KPluginFactory *factory = factoryResult.plugin;
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
    QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")

#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 90)
    const auto qmlKCMResult = factory->create<KQuickAddons::ConfigModule>(pluginKeyword, parent, args2);
#else
    const auto qmlKCMResult = factory->create<KQuickAddons::ConfigModule>(parent, args2);
#endif

    if (qmlKCMResult) {
        std::unique_ptr<KQuickAddons::ConfigModule> kcm(qmlKCMResult);

        if (!kcm->mainUi()) {
            return reportError(ErrorReporting::Inline, i18n("Error loading QML file."), kcm->errorString(), parent);
        }
        return new KCModuleQml(std::move(kcm), parent, args2);
    }

#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 90)
    const auto kcmoduleResult = factory->create<KCModule>(pluginKeyword, parent, args2);
#else
    const auto kcmoduleResult = factory->create<KCModule>(parent, args2);
#endif
    QT_WARNING_POP

    if (kcmoduleResult) {
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

#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 88)
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
KCModule *KCModuleLoader::loadModule(const QString &module, ErrorReporting report, QWidget *parent, const QStringList &args)
{
    return loadModule(KCModuleInfo(module), report, parent, args);
}

KCModule *KCModuleLoader::loadModule(const KCModuleInfo &mod, ErrorReporting report, QWidget *parent, const QStringList &args)
{
    /*
     * Simple libraries as modules are the easiest case:
     *  We just have to load the library and get the module
     *  from the factory.
     */

    if (!mod.isValid()) {
        return reportError(report,
                           i18n("The module %1 could not be found.", mod.moduleName()),
                           i18n("<qt><p>The diagnosis is:<br />The desktop file %1 could not be found.</p></qt>", mod.fileName()),
                           parent);
    }
    if (mod.service() && mod.service()->noDisplay()) {
        return reportError(
            report,
            i18n("The module %1 is disabled.", mod.moduleName()),
            i18n("<qt><p>Either the hardware/software the module configures is not available or the module has been disabled by the administrator.</p></qt>"),
            parent);
    }

    if (!mod.library().isEmpty()) {
        QString error;
        QVariantList args2;
        const QVariantList additionalArgs = mod.property(QStringLiteral("X-KDE-KCM-Args")).toList();
        args2.reserve(args.count() + additionalArgs.count());
        for (const QString &arg : args) {
            args2 << arg;
        }
        args2 << additionalArgs;

        KCModule *module = nullptr;

        const auto result =
            KPluginFactory::instantiatePlugin<KQuickAddons::ConfigModule>(KPluginMetaData(QLatin1String("kcms/") + mod.library()), nullptr, args2);

        if (result) {
            std::unique_ptr<KQuickAddons::ConfigModule> cm(result.plugin);

            if (!cm->mainUi()) {
                return reportError(report, i18n("Error loading QML file."), cm->errorString(), parent);
            }
            module = new KCModuleQml(std::move(cm), parent, args2);
            return module;
        } else {
            // If the error is that the plugin was not found fall through to the compat code
            // Otherwise abort and show the error to the user
            if (result.errorReason != KPluginFactory::INVALID_PLUGIN && result.errorReason != KPluginFactory::INVALID_KPLUGINFACTORY_INSTANTIATION) {
                return reportError(report, i18n("Error loading config module"), result.errorString, parent);
            } else {
                qCDebug(KCMUTILS_LOG) << "Couldn't find plugin" << QLatin1String("kcms/") + mod.library()
                                      << "-- falling back to old-style loading from desktop file";
            }
        }

        if (mod.service()) {
            module = mod.service()->createInstance<KCModule>(parent, args2, &error);
        }
        if (module) {
            return module;
        } else
#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 85)
        {
            // get the create_ function
            QLibrary lib(QPluginLoader(mod.library()).fileName());
            if (lib.load()) {
                KCModule *(*create)(QWidget *, const char *);
                QByteArray factorymethod("create_");
                factorymethod += mod.handle().toLatin1();
                create = reinterpret_cast<KCModule *(*)(QWidget *, const char *)>(lib.resolve(factorymethod.constData()));
                if (create) {
                    return create(parent, mod.handle().toLatin1().constData());
                } else {
                    qCWarning(KCMUTILS_LOG) << "This module has no valid entry symbol at all. The reason could be that it's still using "
                                               "K_EXPORT_COMPONENT_FACTORY with a custom X-KDE-FactoryName which is not supported anymore";
                }
                lib.unload();
            }
        }
#endif
        return reportError(report, error, QString(), parent);
    }

    /*
     * Ok, we could not load the library.
     * Try to run it as an executable.
     * This must not be done when calling from kcmshell, or you'll
     * have infinite recursion
     * (startService calls kcmshell which calls modloader which calls startService...)
     *
     */
    return reportError(report,
                       i18n("The module %1 is not a valid configuration module.", mod.moduleName()),
                       i18n("<qt>The diagnosis is:<br />The desktop file %1 does not specify a library.</qt>", mod.fileName()),
                       parent);
}

void KCModuleLoader::unloadModule(const KCModuleInfo &mod)
{
    // get the library loader instance
    QPluginLoader loader(mod.library());
    loader.unload();
}

bool KCModuleLoader::isDefaults(const KCModuleInfo &mod, const QStringList &args)
{
    std::unique_ptr<KCModuleData> moduleData(loadModuleData(mod, args));
    if (moduleData) {
        return moduleData->isDefaults();
    }

    return true;
}

KCModuleData *KCModuleLoader::loadModuleData(const KCModuleInfo &mod, const QStringList &args)
{
    if (!mod.service() || mod.service()->noDisplay() || mod.library().isEmpty()) {
        return nullptr;
    }

    QVariantList args2(args.cbegin(), args.cend());

    const auto result = KPluginFactory::instantiatePlugin<KCModuleData>(KPluginMetaData(QLatin1String("kcms/") + mod.service()->library()), nullptr, args2);

    if (result) {
        return result.plugin;
    }

    KCModuleData *probe(mod.service()->createInstance<KCModuleData>(nullptr, args2));
    if (probe) {
        return probe;
    }

    return nullptr;
}
#endif
