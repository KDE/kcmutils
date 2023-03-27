/*
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2001 Michael Goffioul <kdeprint@swing.be>
    SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>
    SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

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
#include <QResource>
#include <QUrl>

#include <KLocalizedContext>
#include <KLocalizedString>

#include <memory>

class KQuickConfigModulePrivate
{
public:
    KQuickConfigModulePrivate(KQuickConfigModule *module)
        : _q(module)
    {
    }

    KQuickConfigModule *_q;
    SharedQmlEngine *_engine = nullptr;
    std::shared_ptr<QQmlEngine> passedInEngine;
    QList<QQuickItem *> subPages;
    int _columnWidth = -1;
    int currentIndex = 0;
    QString _errorString;

    static QHash<QObject *, KQuickConfigModule *> s_rootObjects;
    QString getResourcePath(const QString &file)
    {
        return QLatin1String("/kcm/") + _q->metaData().pluginId() + QLatin1String("/") + file;
    }
    QUrl getResourceUrl(const QString &resourcePath)
    {
        return QUrl(QLatin1String("qrc:") + resourcePath);
    }
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

    const QString resourcePath = d->getResourcePath(QStringLiteral("main.qml"));
    if (QResource r(resourcePath); !r.isValid()) {
        d->_errorString = i18n("Could not find resource '%1'", resourcePath);
        qWarning() << "Could not find resource" << resourcePath;
        return nullptr;
    }

    new QQmlFileSelector(d->_engine->engine().get(), this);
    d->_engine->setSource(d->getResourceUrl(resourcePath));
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

    QVariantHash propertyHash;
    for (auto it = propertyMap.begin(), end = propertyMap.end(); it != end; ++it) {
        propertyHash.insert(it.key(), it.value());
    }

    const QString resourcePath = d->getResourcePath(fileName);
    if (QResource r(resourcePath); !r.isValid()) {
        qWarning() << "Requested resource" << resourcePath << "does not exist";
    }
    QObject *object = d->_engine->createObjectFromSource(d->getResourceUrl(resourcePath), d->_engine->rootContext(), propertyHash);

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

QString KQuickConfigModule::errorString() const
{
    return d->_errorString;
}

QQuickItem *KQuickConfigModule::subPage(int index) const
{
    return d->subPages[index];
}

#include "moc_kquickconfigmodule.cpp"