/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcmodulecontainer.h"
#include <kcmutils_debug.h>

#include <QTabWidget>
#include <QVBoxLayout>

#include <kcmoduleinfo.h>
#include <kcmoduleproxy.h>
#include <KService>

typedef QList<KCModuleProxy *> ModuleList;

#if KCMUTILS_BUILD_DEPRECATED_SINCE(5,66)

/***********************************************************************/
class Q_DECL_HIDDEN KCModuleContainer::KCModuleContainerPrivate
{
public:
    KCModuleContainerPrivate(const QStringList &mods)
        : modules(mods)
    {}

    QStringList modules;
    QTabWidget *tabWidget = nullptr;
    KCModule::Buttons buttons;
    QVBoxLayout *topLayout = nullptr;

    /**
     * A list containing KCModuleProxy objects which
     * have changed and must be saved.
     */
    ModuleList changedModules;

    /**
     * A list of all modules which are encapsulated.
     */
    ModuleList allModules;

};
/***********************************************************************/

// The KCModuleContainer is only a wrapper around real KCModules.
/***********************************************************************/
KCModuleContainer::KCModuleContainer(QWidget *parent, const QString &mods)
    : KCModule(parent),
      d(new KCModuleContainerPrivate(QString(mods).remove(QLatin1Char(' ')).split(QLatin1Char(','), Qt::SkipEmptyParts)))
{
    init();
}

KCModuleContainer::KCModuleContainer(QWidget *parent, const QStringList &mods)
    : KCModule(parent),
      d(new KCModuleContainerPrivate(mods))
{
    init();
}

void KCModuleContainer::init()
{
    d->topLayout = new QVBoxLayout(this);
    d->topLayout->setContentsMargins(0, 0, 0, 0);
    d->topLayout->setObjectName(QStringLiteral("topLayout"));
    d->tabWidget = new QTabWidget(this);
    d->tabWidget->setObjectName(QStringLiteral("tabWidget"));
    connect(d->tabWidget, &QTabWidget::currentChanged, this, &KCModuleContainer::tabSwitched);
    d->topLayout->addWidget(d->tabWidget);

    if (!d->modules.isEmpty()) {
        /* Add our modules */
        for (QStringList::const_iterator it = d->modules.constBegin(), total = d->modules.constEnd(); it != total; ++it) {
            addModule((*it));
        }
    }
}

void KCModuleContainer::addModule(const QString &module)
{
    /* In case it doesn't exist we just silently drop it.
     * This allows people to easily extend containers.
     * For example, KCM monitor gamma can be in kdegraphics.
     */
    KService::Ptr service = KService::serviceByDesktopName(module);
    if (!service) {
        // qDebug() << "KCModuleContainer: module '" <<
        // module << "' was not found and thus not loaded" << endl;
        return;
    }

    if (service->noDisplay()) {
        return;
    }

    KCModuleProxy *proxy = new KCModuleProxy(service, d->tabWidget);
    d->allModules.append(proxy);

    proxy->setObjectName(module);

    d->tabWidget->addTab(proxy, QIcon::fromTheme(proxy->moduleInfo().icon()),
                         /* Qt eats ampersands for dinner. But not this time. */
                         proxy->moduleInfo().moduleName().replace(QLatin1Char('&'), QStringLiteral("&&")));

    d->tabWidget->setTabToolTip(d->tabWidget->indexOf(proxy), proxy->moduleInfo().comment());

    connect(proxy, SIGNAL(changed(KCModuleProxy*)), SLOT(moduleChanged(KCModuleProxy*)));

    /* Collect our buttons - we go for the common deliminator */
    setButtons(buttons() | proxy->realModule()->buttons());
}

void KCModuleContainer::tabSwitched(int index)
{
    KCModuleProxy *mod = static_cast<KCModuleProxy *>(d->tabWidget->widget(index));
    setQuickHelp(mod->quickHelp());
    setAboutData(mod->aboutData());
}

void KCModuleContainer::save()
{
    ModuleList list = d->changedModules;
    ModuleList::iterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        (*it)->save();
    }

    Q_EMIT changed(false);

}

void KCModuleContainer::load()
{
    ModuleList list = d->allModules;
    ModuleList::iterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        (*it)->load();
    }

    Q_EMIT changed(false);
}

void KCModuleContainer::defaults()
{
    ModuleList list = d->allModules;
    ModuleList::iterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        (*it)->defaults();
    }

    Q_EMIT changed(true);
}

void KCModuleContainer::moduleChanged(KCModuleProxy *proxy)
{
    d->changedModules.append(proxy);
    if (d->changedModules.isEmpty()) {
        return;
    }

    Q_EMIT changed(true);
}

KCModuleContainer::~KCModuleContainer()
{
    delete d;
}

#endif

/***********************************************************************/

