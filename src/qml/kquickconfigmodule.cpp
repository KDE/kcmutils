/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2001 Michael Goffioul <kdeprint@swing.be>
    SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>
    SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kquickconfigmodule.h"
#include "kabstractconfigmodule.h"
#include "sharedqmlengine_p.h"

#include <QDebug>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlFileSelector>
#include <QQuickItem>
#include <QUrl>

#include <KLocalizedContext>
#include <KLocalizedString>
#include <KPackage/Package>
#include <KPackage/PackageLoader>

#include <memory>

class KQuickConfigModulePrivate
{
public:
    KQuickConfigModulePrivate(KQuickConfigModule *module)
        : _q(module)
    {
    }

    void authStatusChanged(int status);

    KQuickConfigModule *_q;
    SharedQmlEngine *_engine = nullptr;
    std::shared_ptr<QQmlEngine> passedInEngine;
    QList<QQuickItem *> subPages;
    int _columnWidth = -1;
    int currentIndex = 0;
    QString _errorString;

    static QHash<QObject *, KQuickConfigModule *> s_rootObjects;
};

QHash<QObject *, KQuickConfigModule *> KQuickConfigModulePrivate::s_rootObjects = QHash<QObject *, KQuickConfigModule *>();

KQuickConfigModule::KQuickConfigModule(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KAbstractConfigModule(parent, metaData, args)
    , d(new KQuickConfigModulePrivate(this))
{
}

void KQuickConfigModule::setInternalEngine(const std::shared_ptr<QQmlEngine> &engine)
{
    d->passedInEngine = engine;
}

KQuickConfigModule::~KQuickConfigModule()
{
    // in case mainUi was never called
    if (d->_engine) {
        KQuickConfigModulePrivate::s_rootObjects.remove(d->_engine->rootContext());
    }

    delete d;
}

KQuickConfigModule *KQuickConfigModule::qmlAttachedProperties(QObject *object)
{
    // at the moment of the attached object creation, the root item is the only one that hasn't a parent
    // only way to avoid creation of this attached for everybody but the root item
    const QQmlEngine *engine = qmlEngine(object);
    QQmlContext *cont = QQmlEngine::contextForObject(object);

    // Search the qml context that is the "root" for the sharedqmlobject, which
    // is an ancestor of QQmlEngine::contextForObject(object) and the direct child
    // of the engine's root context: we can do this assumption on the internals as
    // we are distributed on the same repo.
    while (cont->parentContext() && cont->parentContext() != engine->rootContext()) {
        cont = cont->parentContext();
    }

    if (!object->parent() && KQuickConfigModulePrivate::s_rootObjects.contains(cont)) {
        return KQuickConfigModulePrivate::s_rootObjects.value(cont);
    } else {
        return nullptr;
    }
}

QQuickItem *KQuickConfigModule::mainUi()
{
    Q_ASSERT(d->passedInEngine);
    if (d->_engine) {
        return qobject_cast<QQuickItem *>(d->_engine->rootObject());
    }

    d->_errorString.clear();
    d->_engine = new SharedQmlEngine(d->passedInEngine, this);

    const QString componentName = metaData().pluginId();
    KQuickConfigModulePrivate::s_rootObjects[d->_engine->rootContext()] = this;
    d->_engine->setTranslationDomain(componentName);
    d->_engine->setInitializationDelayed(true);

    KPackage::Package package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("KPackage/GenericQML"));
    package.setDefaultPackageRoot(QStringLiteral("kpackage/kcms"));
    package.setPath(metaData().pluginId());
    if (metaData().isValid()) {
        package.setMetadata(metaData());
    }

    if (!package.isValid()) {
        d->_errorString = i18n("Invalid KPackage '%1'", componentName);
        qWarning() << "Error loading the module" << componentName << ": invalid KPackage";
        return nullptr;
    }

    if (package.filePath("mainscript").isEmpty()) {
        d->_errorString = i18n("No QML file provided");
        qWarning() << "Error loading the module" << componentName << ": no QML file provided";
        return nullptr;
    }

    new QQmlFileSelector(d->_engine->engine().get(), this);
    d->_engine->setSource(package.fileUrl("mainscript"));
    d->_engine->rootContext()->setContextProperty(QStringLiteral("kcm"), this);
    d->_engine->completeInitialization();

    if (d->_engine->status() != QQmlComponent::Ready) {
        d->_errorString = d->_engine->mainComponent()->errorString();
        return nullptr;
    }

    return qobject_cast<QQuickItem *>(d->_engine->rootObject());
}

void KQuickConfigModule::push(const QString &fileName, const QVariantMap &propertyMap)
{
    // ensure main ui is created
    if (!mainUi()) {
        return;
    }

    // TODO: package as member
    KPackage::Package package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("KPackage/GenericQML"));
    package.setDefaultPackageRoot(QStringLiteral("kpackage/kcms"));
    package.setPath(metaData().pluginId());

    QVariantHash propertyHash;
    for (auto it = propertyMap.begin(), end = propertyMap.end(); it != end; ++it) {
        propertyHash.insert(it.key(), it.value());
    }

    QObject *object = d->_engine->createObjectFromSource(QUrl::fromLocalFile(package.filePath("ui", fileName)), d->_engine->rootContext(), propertyHash);

    QQuickItem *item = qobject_cast<QQuickItem *>(object);
    if (!item) {
        object->deleteLater();
        return;
    }

    d->subPages << item;
    Q_EMIT pagePushed(item);
    Q_EMIT depthChanged(depth());
    setCurrentIndex(d->currentIndex + 1);
}

void KQuickConfigModule::push(QQuickItem *item)
{
    // ensure main ui is created
    if (!mainUi()) {
        return;
    }

    d->subPages << item;
    Q_EMIT pagePushed(item);
    Q_EMIT depthChanged(depth());
    setCurrentIndex(d->currentIndex + 1);
}

void KQuickConfigModule::pop()
{
    if (QQuickItem *page = takeLast()) {
        page->deleteLater();
    }
}

QQuickItem *KQuickConfigModule::takeLast()
{
    if (d->subPages.isEmpty()) {
        return nullptr;
    }
    QQuickItem *page = d->subPages.takeLast();
    Q_EMIT pageRemoved();
    Q_EMIT depthChanged(depth());
    setCurrentIndex(qMin(d->currentIndex, depth() - 1));
    return page;
}

void KQuickConfigModule::showPassiveNotification(const QString &message, const QVariant &timeout, const QString &actionText, const QJSValue &callBack)
{
    Q_EMIT passiveNotificationRequested(message, timeout, actionText, callBack);
}

int KQuickConfigModule::columnWidth() const
{
    return d->_columnWidth;
}

void KQuickConfigModule::setColumnWidth(int width)
{
    if (d->_columnWidth == width) {
        return;
    }

    d->_columnWidth = width;
    Q_EMIT columnWidthChanged(width);
}

int KQuickConfigModule::depth() const
{
    return d->subPages.count() + 1;
}

void KQuickConfigModule::setCurrentIndex(int index)
{
    if (index < 0 || index > d->subPages.count() || index == d->currentIndex) {
        return;
    }

    d->currentIndex = index;

    Q_EMIT currentIndexChanged(index);
}

int KQuickConfigModule::currentIndex() const
{
    return d->currentIndex;
}

std::shared_ptr<QQmlEngine> KQuickConfigModule::engine() const
{
    return d->_engine->engine();
}

QQmlComponent::Status KQuickConfigModule::status() const
{
    if (!d->_engine) {
        return QQmlComponent::Null;
    }

    return d->_engine->status();
}

QString KQuickConfigModule::errorString() const
{
    return d->_errorString;
}

QQuickItem *KQuickConfigModule::subPage(int index) const
{
    return d->subPages[index];
}

#include "moc_kquickconfigmodule.cpp"
