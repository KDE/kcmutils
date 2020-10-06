/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>
    SPDX-FileCopyrightText: 2003 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcmoduleproxy.h"
#include "kcmoduleproxy_p.h"

#include <QApplication>
#include <QLayout>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusServiceWatcher>

#include <KAboutData>
#include <kcmoduleinfo.h>

#include <KLocalizedString>

#include <kcmoduleloader.h>
#include <kcmoduleqml_p.h>

#include <KColorScheme>

#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 82)
#include "ksettingswidgetadaptor.h"
#endif

#include <kcmutils_debug.h>

/*
 TODO:

 - Resizing horizontally is contrained; minimum size is set somewhere.
    It appears to be somehow derived from the module's size.

 - Prettify: set icon in KCMultiDialog.

 */
/***************************************************************/
KCModule *KCModuleProxy::realModule() const
{
    Q_D(const KCModuleProxy);
    /*
     * Note, don't call any function that calls realModule() since
     * that leads to an infinite loop.
     */

    /* Already loaded */
    if (!d->kcm) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        const_cast<KCModuleProxyPrivate *>(d)->loadModule();
        QApplication::restoreOverrideCursor();
    }
    return d->kcm;
}

void KCModuleProxyPrivate::loadModule()
{
    if (!topLayout) {
        topLayout = new QVBoxLayout(parent);

        QString name = modInfo.handle();
        name.replace(QLatin1Char('-'), QLatin1Char('_')); // hyphen is not allowed in dbus, only [A-Z][a-z][0-9]_
        name.replace(QLatin1Char('/'), QLatin1Char('_')); // same goes for '/'
        dbusService = QLatin1String("org.kde.internal.KSettingsWidget_") + name;

        // for dbus path, we also need to convert '.' characters
        name.replace(QLatin1Char('.'), QLatin1Char('_'));
        dbusPath = QLatin1String("/internal/KSettingsWidget/") + name;
    }

    const bool canRegisterService = QDBusConnection::sessionBus().registerService(dbusService);

    if (!canRegisterService) {
        /* We didn't get the name we requested, because it's already taken, */
        /* Figure out the name of where the module is already loaded */
        QDBusInterface proxy(dbusService, dbusPath, QStringLiteral("org.kde.internal.KSettingsWidget"));
        QDBusReply<QString> reply = proxy.call(QStringLiteral("applicationName"));
        /* If we get a valid application name, the module is already loaded */
        if (reply.isValid()) {
            auto *watcher = new QDBusServiceWatcher(parent);
            watcher->addWatchedService(dbusService);
            watcher->setConnection(QDBusConnection::sessionBus());
            watcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
            QObject::connect(watcher,
                             &QDBusServiceWatcher::serviceOwnerChanged,
                             parent,
                             [this](const QString &serviceName, const QString &oldOwner, const QString &newOwner) {
                                 _k_ownerChanged(serviceName, oldOwner, newOwner);
                             });

            kcm = KCModuleLoader::reportError(KCModuleLoader::Inline,
                                              i18nc("Argument is application name", "This configuration section is already opened in %1", reply.value()),
                                              QStringLiteral(" "),
                                              parent);
            topLayout->addWidget(kcm);
            return;
        }
    }

    // qDebug() << "Module not already loaded, loading module " << modInfo.moduleName() << " from library " << modInfo.library() << " using symbol " << modInfo.handle();
    kcm = KCModuleLoader::loadModule(modInfo, KCModuleLoader::Inline, parent, args);

    QObject::connect(kcm, &KCModule::changed, parent, [this](bool state) {
        _k_moduleChanged(state);
    });
    QObject::connect(kcm, &KCModule::defaulted, parent, [this](bool state) {
        _k_moduleDefaulted(state);
    });
    QObject::connect(kcm, &KCModule::destroyed, parent, [this]() {
        _k_moduleDestroyed();
    });
    QObject::connect(kcm, &KCModule::quickHelpChanged, parent, &KCModuleProxy::quickHelpChanged);
    parent->setWhatsThis(kcm->quickHelp());

    if (kcm->layout()) {
        kcm->layout()->setContentsMargins(0, 0, 0, 0);
    }
    if (qobject_cast<KCModuleQml *>(kcm)) {
        topLayout->setContentsMargins(0, 0, 0, 0);
    }

    topLayout->addWidget(kcm);

#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 82)
    if (!modInfo.handle().isEmpty()) {
        QDBusConnection::sessionBus().registerObject(dbusPath, new KSettingsWidgetAdaptor(parent), QDBusConnection::ExportAllSlots);
    }
#endif
}

void KCModuleProxyPrivate::_k_ownerChanged(const QString &service, const QString &oldOwner, const QString &)
{
    if (service == dbusService && !oldOwner.isEmpty()) {
        // Violence: Get rid of KCMError & CO, so that
        // realModule() attempts to reload the module
        delete kcm;
        kcm = nullptr;
        Q_Q(KCModuleProxy);
        q->realModule();

        Q_ASSERT(kcm);
        kcm->show();
    }
}

void KCModuleProxy::showEvent(QShowEvent *ev)
{
    Q_D(KCModuleProxy);

    (void)realModule();

    /* We have no kcm, if we're in root mode */
    if (d->kcm) {
        d->kcm->showEvent(ev);
    }

    QWidget::showEvent(ev);
}

KCModuleProxy::~KCModuleProxy()
{
    deleteClient();
    KCModuleLoader::unloadModule(moduleInfo());

    delete d_ptr;
}

void KCModuleProxy::deleteClient()
{
    Q_D(KCModuleProxy);
    delete d->kcm;
    d->kcm = nullptr;
}

void KCModuleProxyPrivate::_k_moduleChanged(bool c)
{
    if (changed == c) {
        return;
    }

    Q_Q(KCModuleProxy);
    changed = c;
    Q_EMIT q->changed(c);
    Q_EMIT q->changed(q);
}

void KCModuleProxyPrivate::_k_moduleDefaulted(bool d)
{
    if (defaulted == d) {
        return;
    }

    Q_Q(KCModuleProxy);
    defaulted = d;
    Q_EMIT q->changed(changed);
    Q_EMIT q->changed(q);
}

void KCModuleProxyPrivate::_k_moduleDestroyed()
{
    kcm = nullptr;
}

KCModuleProxy::KCModuleProxy(const KService::Ptr &service, QWidget *parent, const QStringList &args)
    : QWidget(parent)
    , d_ptr(new KCModuleProxyPrivate(this, KCModuleInfo(service), args))
{
    d_ptr->q_ptr = this;
}

KCModuleProxy::KCModuleProxy(const KCModuleInfo &info, QWidget *parent, const QStringList &args)
    : QWidget(parent)
    , d_ptr(new KCModuleProxyPrivate(this, info, args))
{
    d_ptr->q_ptr = this;
}

KCModuleProxy::KCModuleProxy(const QString &serviceName, QWidget *parent, const QStringList &args)
    : QWidget(parent)
    , d_ptr(new KCModuleProxyPrivate(this, KCModuleInfo(serviceName), args))
{
    d_ptr->q_ptr = this;
}

void KCModuleProxy::load()
{
    Q_D(KCModuleProxy);
    if (realModule()) {
        d->kcm->load();
        d->_k_moduleChanged(false);
    }
}

void KCModuleProxy::save()
{
    Q_D(KCModuleProxy);
    if (d->changed && realModule()) {
        d->kcm->save();
        d->_k_moduleChanged(false);
    }
}

void KCModuleProxy::defaults()
{
    Q_D(KCModuleProxy);
    if (realModule()) {
        d->kcm->defaults();
    }
}

QString KCModuleProxy::quickHelp() const
{
    return realModule() ? realModule()->quickHelp() : QString();
}

const KAboutData *KCModuleProxy::aboutData() const
{
    return realModule() ? realModule()->aboutData() : nullptr;
}

KCModule::Buttons KCModuleProxy::buttons() const
{
    if (realModule()) {
        return realModule()->buttons();
    }
    return KCModule::Buttons(KCModule::Help | KCModule::Default | KCModule::Apply);
}

bool KCModuleProxy::changed() const
{
    Q_D(const KCModuleProxy);
    return d->changed;
}

bool KCModuleProxy::defaulted() const
{
    Q_D(const KCModuleProxy);
    return d->defaulted;
}

KCModuleInfo KCModuleProxy::moduleInfo() const
{
    Q_D(const KCModuleProxy);
    return d->modInfo;
}

QString KCModuleProxy::dbusService() const
{
    Q_D(const KCModuleProxy);
    return d->dbusService;
}

QString KCModuleProxy::dbusPath() const
{
    Q_D(const KCModuleProxy);
    return d->dbusPath;
}

QSize KCModuleProxy::minimumSizeHint() const
{
    return QWidget::minimumSizeHint();
}

void KCModuleProxy::setDefaultsIndicatorsVisible(bool show)
{
    Q_D(KCModuleProxy);
    if (realModule()) {
        d->kcm->setDefaultsIndicatorsVisible(show);
    }
}

// X void KCModuleProxy::emitQuickHelpChanged()
// X {
// X     emit quickHelpChanged();
// X }

/***************************************************************/
#include "moc_kcmoduleproxy.cpp"
