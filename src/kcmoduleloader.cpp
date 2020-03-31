/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
    SPDX-FileCopyrightText: 2003, 2004, 2006 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcmoduleloader.h"
#include "kcmoduledata.h"
#include "kcmoduleqml_p.h"
#include <kcmutils_debug.h>

#include <QLabel>
#include <QLibrary>
#include <QVBoxLayout>

#include <KPluginInfo>
#include <KPluginLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KAboutData>

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
/***************************************************************/

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
                           i18n("The module %1 could not be found.",
                                mod.moduleName()), i18n("<qt><p>The diagnosis is:<br />The desktop file %1 could not be found.</p></qt>", mod.fileName()), parent);
    }
    if (mod.service() && mod.service()->noDisplay()) {
        return reportError(report, i18n("The module %1 is disabled.", mod.moduleName()),
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

        KPluginLoader loader(KPluginLoader::findPlugin(QLatin1String("kcms/") + mod.library()));
        KPluginFactory* factory = loader.factory();
        if (!factory) {
            // KF6 TODO: make this a warning, and remove mention of fallback
            qCDebug(KCMUTILS_LOG) << "Couldn't load plugin" << QLatin1String("kcms/") + mod.library() << ":" << loader.errorString() << " -- falling back to old-style loading from desktop file";
        } else {
            std::unique_ptr<KQuickAddons::ConfigModule> cm(factory->create<KQuickAddons::ConfigModule>(nullptr, args2));
            if (!cm) {
                qCWarning(KCMUTILS_LOG) << "Error creating object from plugin" << loader.fileName();
            } else {
                if (!cm->mainUi()) {
                    return reportError(report, i18n("Error loading QML file."), cm->errorString(), parent);
                }
                module = new KCModuleQml(std::move(cm), parent, args2);
                return module;
            }
        }

        // KF6 TODO: remove this compat block
        if (mod.service()) {
            module = mod.service()->createInstance<KCModule>(parent, args2, &error);
        }
        if (module) {
            return module;
        } else
//#ifndef NDEBUG
        {
            // KF6 TODO: remove this old compat block
            // get the create_ function
            QLibrary lib(KPluginLoader::findPlugin(mod.library()));
            if (lib.load()) {
                KCModule *(*create)(QWidget *, const char *);
                QByteArray factorymethod("create_");
                factorymethod += mod.handle().toLatin1();
                create = reinterpret_cast<KCModule *(*)(QWidget *, const char *)>(lib.resolve(factorymethod.constData()));
                if (create) {
                    return create(parent, mod.handle().toLatin1().constData());
                } else {
                    qCWarning(KCMUTILS_LOG) << "This module has no valid entry symbol at all. The reason could be that it's still using K_EXPORT_COMPONENT_FACTORY with a custom X-KDE-FactoryName which is not supported anymore";
                }
                lib.unload();
            }
        }
//#endif // NDEBUG
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
                       i18n("<qt>The diagnosis is:<br />The desktop file %1 does not specify a library.</qt>", mod.fileName()), parent);
}

void KCModuleLoader::unloadModule(const KCModuleInfo &mod)
{
    // get the library loader instance
    KPluginLoader loader(mod.library());
    loader.unload();
}

bool KCModuleLoader::isDefaults(const KCModuleInfo &mod, const QStringList &args)
{
    if (!mod.service() || mod.service()->noDisplay() || mod.library().isEmpty()) {
        return true;
    }

    QVariantList args2(args.cbegin(), args.cend());

    KPluginLoader loader(KPluginLoader::findPlugin(QLatin1String("kcms/") + mod.service()->library()));
    KPluginFactory* factory = loader.factory();
    if (factory) {
        std::unique_ptr<KCModuleData> probe(factory->create<KCModuleData>(nullptr, args2));
        if (probe) {
            return probe->isDefaults();
        }
    }

    std::unique_ptr<KCModuleData> probe(mod.service()->createInstance<KCModuleData>(nullptr, args2));
    if (probe) {
        return probe->isDefaults();
    }

    return true;
}


KCModule *KCModuleLoader::reportError(ErrorReporting report, const QString &text,
                                      const QString &details, QWidget *parent)
{
    QString realDetails = details;
    if (realDetails.isNull()) {
        realDetails = i18n("<qt><p>Possible reasons:<ul><li>An error occurred during your last "
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

