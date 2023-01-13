/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2001 Michael Goffioul <kdeprint@swing.be>
    SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>
    SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcmodule.h"
#include "kcmutils_debug.h"

#include <KConfigDialogManager>
#include <KConfigSkeleton>
#include <KLocalizedContext>
#include <KPluginMetaData>
#if KCMUTILS_WITH_KAUTH
#include <KAuth/ExecuteJob>
#endif

class KCModulePrivate
{
public:
    KCModulePrivate()
        : _buttons(KCModule::Help | KCModule::Default | KCModule::Apply)
        , _firstshow(true)
        , _needsAuthorization(false)
        , _unmanagedWidgetChangeState(false)
        , _unmanagedWidgetDefaultState(false)
        , _unmanagedWidgetDefaultStateCalled(false)
    {
    }

    void authStatusChanged(int status);

    KCModule::Buttons _buttons;
    QList<KConfigDialogManager *> managers;
    bool _firstshow : 1;

    bool _needsAuthorization : 1;
#if KCMUTILS_WITH_KAUTH
    KAuth::Action _authAction;
#endif

    // this member is used to record the state on non-automatically
    // managed widgets, allowing for mixed KConfigXT-drive and manual
    // widgets to coexist peacefully and do the correct thing with
    // the changed(bool) signal
    bool _unmanagedWidgetChangeState : 1;
    bool _unmanagedWidgetDefaultState : 1;
    bool _unmanagedWidgetDefaultStateCalled : 1;
};

KCModule::KCModule(QWidget *parent, const KPluginMetaData &data, const QVariantList &)
    : QWidget(parent)
    , d(new KCModulePrivate)
{
}

void KCModule::showEvent(QShowEvent *ev)
{
    if (d->_firstshow) {
        d->_firstshow = false;
        QMetaObject::invokeMethod(this, &KCModule::load, Qt::QueuedConnection);
        auto changedFunc = [this]() {
            changed(false);
        };
        QMetaObject::invokeMethod(this, changedFunc, Qt::QueuedConnection);
    }

    QWidget::showEvent(ev);
}


KConfigDialogManager *KCModule::addConfig(KCoreConfigSkeleton *config, QWidget *widget)
{
    KConfigDialogManager *manager = new KConfigDialogManager(widget, config);
    manager->setObjectName(objectName());
    connect(manager, &KConfigDialogManager::widgetModified, this, &KCModule::widgetChanged);
    connect(manager, &QObject::destroyed, this, [this, manager]() {
        d->managers.removeOne(manager);
    });
    d->managers.append(manager);
    return manager;
}

#if KCMUTILS_WITH_KAUTH
void KCModule::setAuthAction(const KAuth::Action &action)
{
    if (!action.isValid()) {
        qCWarning(KCMUTILS_LOG) << "Auth action" << action.name() << "is invalid";
        d->_needsAuthorization = false;
        return;
    }
    d->_authAction = action;
    d->_needsAuthorization = true;
    d->_authAction.setParentWidget(this);
    authStatusChanged(d->_authAction.status());
}

KAuth::Action KCModule::authAction() const
{
    return d->_authAction;
}

void KCModule::authStatusChanged(KAuth::Action::AuthStatus status)
{
    switch (status) {
    case KAuth::Action::AuthorizedStatus:
        setUseRootOnlyMessage(false);
        break;
    case KAuth::Action::AuthRequiredStatus:
        setUseRootOnlyMessage(true);
        setRootOnlyMessage(i18n("You will be asked to authenticate before saving"));
        break;
    default:
        setUseRootOnlyMessage(true);
        setRootOnlyMessage(i18n("You are not allowed to save the configuration"));
        break;
    }

    qCDebug(KCMUTILS_LOG) << useRootOnlyMessage();
}
#endif

KCModule::~KCModule()
{
    qDeleteAll(d->managers);
    d->managers.clear();
}

void KCModule::load()
{
    for (KConfigDialogManager *manager : std::as_const(d->managers)) {
        manager->updateWidgets();
    }
    widgetChanged();
}

void KCModule::save()
{
    for (KConfigDialogManager *manager : std::as_const(d->managers)) {
        manager->updateSettings();
    }
    setNeedsSave(false);
}

void KCModule::defaults()
{
    for (KConfigDialogManager *manager : std::as_const(d->managers)) {
        manager->updateWidgetsDefault();
    }
}

void KCModule::widgetChanged()
{
    Q_EMIT changed(d->_unmanagedWidgetChangeState || managedWidgetChangeState());
    if (d->_unmanagedWidgetDefaultStateCalled) {
        Q_EMIT defaulted(d->_unmanagedWidgetDefaultState && managedWidgetDefaultState());
    } else {
        Q_EMIT defaulted(!d->managers.isEmpty() && managedWidgetDefaultState());
    }
}

bool KCModule::managedWidgetChangeState() const
{
    for (KConfigDialogManager *manager : std::as_const(d->managers)) {
        if (manager->hasChanged()) {
            return true;
        }
    }

    return false;
}

bool KCModule::managedWidgetDefaultState() const
{
    for (KConfigDialogManager *manager : std::as_const(d->managers)) {
        if (!manager->isDefault()) {
            return false;
        }
    }

    return true;
}

void KCModule::unmanagedWidgetChangeState(bool changed)
{
    d->_unmanagedWidgetChangeState = changed;
    widgetChanged();
}

void KCModule::unmanagedWidgetDefaultState(bool defaulted)
{
    d->_unmanagedWidgetDefaultStateCalled = true;
    d->_unmanagedWidgetDefaultState = defaulted;
    widgetChanged();
}

void KCModule::setRootOnlyMessage(const QString &message)
{
    d->_rootOnlyMessage = message;
    Q_EMIT rootOnlyMessageChanged(d->_useRootOnlyMessage, d->_rootOnlyMessage);
}

QString KCModule::rootOnlyMessage() const
{
    return d->_rootOnlyMessage;
}

void KCModule::setUseRootOnlyMessage(bool on)
{
    d->_useRootOnlyMessage = on;
    Q_EMIT rootOnlyMessageChanged(d->_useRootOnlyMessage, d->_rootOnlyMessage);
}

bool KCModule::useRootOnlyMessage() const
{
    return d->_useRootOnlyMessage;
}

void KCModule::markAsChanged()
{
    Q_EMIT changed(true);
}

void KCModule::setQuickHelp(const QString &help)
{
    d->_quickHelp = help;
    Q_EMIT quickHelpChanged();
}

QString KCModule::quickHelp() const
{
    return d->_quickHelp;
}

QList<KConfigDialogManager *> KCModule::configs() const
{
    return d->managers;
}
