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
#include "kcmutils_debug.h"

#include <QDebug>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlFileSelector>
#include <QQuickItem>
#include <QResource>
#include <QUrl>

#include <KLocalizedQmlContext>
#include <KLocalizedString>
#include <KRuntimePlatform>

class KQuickConfigModulePrivate
{
public:
    KQuickConfigModulePrivate(KQuickConfigModule *module)
        : q(module)
    {
    }

    KQuickConfigModule *q;
    QQuickItem *rootItem = nullptr;
    QQmlContext *qmlContext = nullptr;
    std::shared_ptr<QQmlEngine> passedInEngine;
    QList<QQuickItem *> subPages;
    int columnWidth = -1;
    int currentIndex = 0;
    QString errorString;

    static QHash<QQmlContext *, KQuickConfigModule *> rootObjects;

    QString getResourcePath(const QString &file)
    {
        return QLatin1String("/kcm/") + q->metaData().pluginId() + QLatin1String("/") + file;
    }
    QUrl getResourceUrl(const QString &resourcePath)
    {
        return QUrl(QLatin1String("qrc:") + resourcePath);
    }
};

QHash<QQmlContext *, KQuickConfigModule *> KQuickConfigModulePrivate::rootObjects = QHash<QQmlContext *, KQuickConfigModule *>();

KQuickConfigModule::KQuickConfigModule(QObject *parent, const KPluginMetaData &metaData)
    : KAbstractConfigModule(parent, metaData)
    , d(new KQuickConfigModulePrivate(this))
{
}

void KQuickConfigModule::setInternalEngine(const std::shared_ptr<QQmlEngine> &engine)
{
    d->passedInEngine = engine;
}

KQuickConfigModule::~KQuickConfigModule()
{
    for (QQuickItem *subPage : std::as_const(d->subPages)) {
        subPage->deleteLater();
    }

    // in case mainUi was never called
    if (d->rootItem) {
        KQuickConfigModulePrivate::rootObjects.remove(d->qmlContext);
        delete d->rootItem;
        delete d->qmlContext;
    }
}

KQuickConfigModule *KQuickConfigModule::qmlAttachedProperties(QObject *object)
{
    // at the moment of the attached object creation, the root item is the only one that hasn't a parent
    // only way to avoid creation of this attached for everybody but the root item
    const QQmlEngine *engine = qmlEngine(object);
    QQmlContext *ctx = qmlContext(object);

    // Search the qml context that is the "root" for the sharedqmlobject,
    // which is an ancestor of qmlContext(object) and the direct child of the
    // engine's root context: we can do this assumption on the internals as
    // we are distributed on the same repo.
    while (ctx->parentContext() && ctx->parentContext() != engine->rootContext()) {
        ctx = ctx->parentContext();
    }

    if (!object->parent() && KQuickConfigModulePrivate::rootObjects.contains(ctx)) {
        return KQuickConfigModulePrivate::rootObjects.value(ctx);
    } else {
        return nullptr;
    }
}

QQuickItem *KQuickConfigModule::mainUi()
{
    Q_ASSERT(d->passedInEngine);
    if (d->rootItem) {
        return d->rootItem;
    }

    d->qmlContext = new QQmlContext(d->passedInEngine.get());
    auto i18nContext = new KLocalizedQmlContext(d->qmlContext);
    d->qmlContext->setContextObject(i18nContext);
    QQmlEngine::setContextForObject(i18nContext, d->qmlContext);

    d->errorString.clear();

    KQuickConfigModulePrivate::rootObjects[d->qmlContext] = this;

    const QString componentName = metaData().pluginId();
    i18nContext->setTranslationDomain(componentName);

    QString resourcePath = d->getResourcePath(QStringLiteral("main.qml"));

    qCDebug(KCMUTILS_LOG) << "Current platform is " << KRuntimePlatform::runtimePlatform();

    const auto platforms = KRuntimePlatform::runtimePlatform();
    for (const QString &platform : platforms) {
        const QString platformResourcePath = d->getResourcePath(QString(QStringLiteral("main_%1.qml")).arg(platform));
        if (QResource r(platformResourcePath); r.isValid()) {
            qCDebug(KCMUTILS_LOG) << "Found platform-specific QML main file at" << platformResourcePath;
            resourcePath = std::move(platformResourcePath);
            break;
        }
    }

    if (QResource r(resourcePath); !r.isValid()) {
        d->errorString = i18n("Could not find resource '%1'", resourcePath);
        qCWarning(KCMUTILS_LOG) << "Could not find resource" << resourcePath;
        return nullptr;
    }

    new QQmlFileSelector(d->passedInEngine.get(), this);

    QQmlComponent component(d->passedInEngine.get(), d->getResourceUrl(resourcePath));
    d->rootItem = qobject_cast<QQuickItem *>(component.beginCreate(d->qmlContext));
    d->qmlContext->setContextProperty(QStringLiteral("kcm"), this);
    component.completeCreate();

    if (component.isError()) {
        d->errorString = component.errorString();
        return nullptr;
    }

    Q_EMIT mainUiReady();

    return d->rootItem;
}

void KQuickConfigModule::push(const QString &fileName, const QVariantMap &initialProperties)
{
    // ensure main ui is created
    if (!mainUi()) {
        return;
    }

    const QString resourcePath = d->getResourcePath(fileName);
    if (QResource r(resourcePath); !r.isValid()) {
        qCWarning(KCMUTILS_LOG) << "Requested resource" << resourcePath << "does not exist";
    }

    QQmlComponent component(d->passedInEngine.get(), d->getResourceUrl(resourcePath));
    auto object = component.createWithInitialProperties(initialProperties, d->qmlContext);

    QQuickItem *item = qobject_cast<QQuickItem *>(object);
    if (!item) {
        if (object) {
            object->deleteLater();
        }
        return;
    }

    item->setParentItem(d->rootItem);

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
    return d->columnWidth;
}

void KQuickConfigModule::setColumnWidth(int width)
{
    if (d->columnWidth == width) {
        return;
    }

    d->columnWidth = width;
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
    return d->passedInEngine;
}

QString KQuickConfigModule::errorString() const
{
    return d->errorString;
}

QQuickItem *KQuickConfigModule::subPage(int index) const
{
    return d->subPages[index];
}

#include "moc_kquickconfigmodule.cpp"
