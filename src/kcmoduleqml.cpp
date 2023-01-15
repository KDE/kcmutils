/*
    This file is part of the KDE Project
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcmoduleqml_p.h"

#include <Kirigami2/kirigami/sharedqmlengine.h>
#include <QQuickItem>
#include <QQuickWidget>
#include <QQuickWindow>
#include <QVBoxLayout>

#include <KAboutData>
#include <KLocalizedContext>
#include <KPageWidget>
#include <Kirigami/SharedQmlEngine>

#include "qml/configmodule.h"

#include <kcmutils_debug.h>

class KCModuleQmlPrivate
{
public:
    KCModuleQmlPrivate(std::unique_ptr<KQuickAddons::ConfigModule> cm, KCModuleQml *qq)
        : q(qq)
        , configModule(std::move(cm))
    {
    }

    ~KCModuleQmlPrivate()
    {
    }

    void syncCurrentIndex()
    {
        if (!configModule || !pageRow) {
            return;
        }

        configModule->setCurrentIndex(pageRow->property("currentIndex").toInt());
    }

    KCModuleQml *q;
    QQuickWindow *quickWindow = nullptr;
    QQuickWidget *quickWidget = nullptr;
    QQuickItem *rootPlaceHolder = nullptr;
    QQuickItem *pageRow = nullptr;
    std::unique_ptr<KQuickAddons::ConfigModule> configModule;
    Kirigami::SharedQmlEngine *engine = nullptr;
};

class QmlConfigModuleWidget : public QWidget
{
public:
    QmlConfigModuleWidget(KCModuleQml *module, QWidget *parent)
        : QWidget(parent)
        , m_module(module)
    {
        setFocusPolicy(Qt::StrongFocus);
    }

    void focusInEvent(QFocusEvent *event) override
    {
        if (event->reason() == Qt::TabFocusReason) {
            m_module->d->rootPlaceHolder->nextItemInFocusChain(true)->forceActiveFocus(Qt::TabFocusReason);
        } else if (event->reason() == Qt::BacktabFocusReason) {
            m_module->d->rootPlaceHolder->nextItemInFocusChain(false)->forceActiveFocus(Qt::BacktabFocusReason);
        }
    }

    QSize sizeHint() const override
    {
        if (!m_module->d->rootPlaceHolder) {
            return QSize();
        }

        return QSize(m_module->d->rootPlaceHolder->implicitWidth(), m_module->d->rootPlaceHolder->implicitHeight());
    }

    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == m_module->d->rootPlaceHolder && event->type() == QEvent::FocusIn) {
            auto focusEvent = static_cast<QFocusEvent *>(event);
            if (focusEvent->reason() == Qt::TabFocusReason) {
                QWidget *w = m_module->d->quickWidget->nextInFocusChain();
                while (!w->isEnabled() || !(w->focusPolicy() & Qt::TabFocus)) {
                    w = w->nextInFocusChain();
                }
                w->setFocus(Qt::TabFocusReason); // allow tab navigation inside the qquickwidget
                return true;
            } else if (focusEvent->reason() == Qt::BacktabFocusReason) {
                QWidget *w = m_module->d->quickWidget->previousInFocusChain();
                while (!w->isEnabled() || !(w->focusPolicy() & Qt::TabFocus)) {
                    w = w->previousInFocusChain();
                }
                w->setFocus(Qt::BacktabFocusReason);
                return true;
            }
        }
        return QWidget::eventFilter(watched, event);
    }

private:
    KCModuleQml *m_module;
};

KCModuleQml::KCModuleQml(std::unique_ptr<KQuickAddons::ConfigModule> configModule, QWidget *parent, const QVariantList &args)
    : KCModule(new QmlConfigModuleWidget(this, parent), {}, args)
    , d(new KCModuleQmlPrivate(std::move(configModule), this))
{
    connect(d->configModule.get(), &KQuickAddons::ConfigModule::quickHelpChanged, this, &KCModuleQml::quickHelpChanged);
    // HACK:Here is important those two enums keep having the exact same values
    // but the kdeclarative one can't use the KCModule's enum
    setButtons((KCModule::Buttons)(int)d->configModule->buttons());
    connect(d->configModule.get(), &KQuickAddons::ConfigModule::buttonsChanged, this, [=] {
        setButtons((KCModule::Buttons)(int)d->configModule->buttons());
    });

    setNeedsSave(d->configModule->needsSave());
    connect(d->configModule.get(), &KQuickAddons::ConfigModule::needsSaveChanged, this, [=] {
        setNeedsSave(d->configModule->needsSave());
    });
    connect(d->configModule.get(), &KQuickAddons::ConfigModule::representsDefaultsChanged, this, [=] {
        setRepresentsDefaults(d->configModule->representsDefaults());
    });

    setNeedsAuthorization(d->configModule->needsAuthorization());
    connect(d->configModule.get(), &KQuickAddons::ConfigModule::needsAuthorizationChanged, this, [=] {
        setNeedsAuthorization(d->configModule->needsAuthorization());
    });

    setRootOnlyMessage(d->configModule->rootOnlyMessage());
    setUseRootOnlyMessage(d->configModule->useRootOnlyMessage());
    connect(d->configModule.get(), &KQuickAddons::ConfigModule::rootOnlyMessageChanged, this, [=] {
        setRootOnlyMessage(d->configModule->rootOnlyMessage());
    });
    connect(d->configModule.get(), &KQuickAddons::ConfigModule::useRootOnlyMessageChanged, this, [=] {
        setUseRootOnlyMessage(d->configModule->useRootOnlyMessage());
    });

#if KCMUTILS_WITH_KAUTH
    if (!d->configModule->authActionName().isEmpty()) {
        setAuthAction(KAuth::Action(d->configModule->authActionName()));
    }
    connect(d->configModule.get(), &KQuickAddons::ConfigModule::authActionNameChanged, this, [=] {
        setAuthAction(d->configModule->authActionName());
    });
#endif

    connect(this, &KCModule::defaultsIndicatorsVisibleChanged, d->configModule.get(), &KQuickAddons::ConfigModule::defaultsIndicatorsVisibleChanged);

    // Build the UI
    QVBoxLayout *layout = new QVBoxLayout(widget());
    layout->setContentsMargins(0, 0, 0, 0);

    d->engine = Kirigami::SharedQmlEngine::create(nullptr, this);
    d->quickWidget = new QQuickWidget(d->engine->engine().get(), widget());
    d->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    d->quickWidget->setFocusPolicy(Qt::StrongFocus);
    d->quickWidget->setAttribute(Qt::WA_AlwaysStackOnTop, true);
    d->quickWindow = d->quickWidget->quickWindow();
    d->quickWindow->setColor(Qt::transparent);

    QQmlComponent *component = new QQmlComponent(d->engine->engine().get(), this);
    // this has activeFocusOnTab to notice when the navigation wraps
    // around, so when we need to go outside and inside
    // pushPage/popPage are needed as push of StackView can't be directly invoked from c++
    // because its parameters are QQmlV4Function which is not public.
    // The managers of onEnter/ReturnPressed are a workaround of
    // Qt bug https://bugreports.qt.io/browse/QTBUG-70934
    // clang-format off
    component->setData(QByteArrayLiteral(R"(
import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import org.kde.kirigami 2.14 as Kirigami

Kirigami.ApplicationItem {
    //force it to *never* try to resize itself
    width: Window.width

    implicitWidth: pageStack.implicitWidth
    implicitHeight: pageStack.implicitHeight

    activeFocusOnTab: true
    controlsVisible: false

    property QtObject kcm

    ToolButton {
        id:toolButton
        visible: false
        icon.name: "go-previous"
    }

    pageStack.separatorVisible: false
    pageStack.globalToolBar.preferredHeight: toolButton.implicitHeight + Kirigami.Units.smallSpacing * 2
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.ToolBar
    pageStack.globalToolBar.showNavigationButtons: pageStack.currentIndex > 0 ? Kirigami.ApplicationHeaderStyle.ShowBackButton : Kirigami.ApplicationHeaderStyle.NoNavigationButtons

    pageStack.columnView.columnResizeMode: pageStack.items.length > 0 && pageStack.items[0].Kirigami.ColumnView.fillWidth
        ? Kirigami.ColumnView.SingleColumn
        : Kirigami.ColumnView.FixedColumns

    pageStack.defaultColumnWidth: kcm && kcm.columnWidth > 0 ? kcm.columnWidth : Kirigami.Units.gridUnit * 20

    Keys.onReturnPressed: event => {
        event.accepted = true
    }
    Keys.onEnterPressed: event => {
        event.accepted = true
    }
}
    )"), QUrl(QStringLiteral("kcmutils/kcmmoduleqml.cpp")));
    // clang-format on

    d->rootPlaceHolder = qobject_cast<QQuickItem *>(component->create());
    if (!d->rootPlaceHolder) {
        qCCritical(KCMUTILS_LOG) << component->errors();
        qFatal("Failed to initialize KCModuleQML");
    }
    d->rootPlaceHolder->setProperty("kcm", QVariant::fromValue(d->configModule.get()));
    d->rootPlaceHolder->installEventFilter(this);
    d->quickWidget->setContent(QUrl(), component, d->rootPlaceHolder);

    d->pageRow = d->rootPlaceHolder->property("pageStack").value<QQuickItem *>();
    if (d->pageRow) {
        d->pageRow->setProperty("initialPage", QVariant::fromValue(d->configModule->mainUi()));

        for (int i = 0; i < d->configModule->depth() - 1; i++) {
            QMetaObject::invokeMethod(d->pageRow,
                                      "push",
                                      Qt::DirectConnection,
                                      Q_ARG(QVariant, QVariant::fromValue(d->configModule->subPage(i))),
                                      Q_ARG(QVariant, QVariant()));
        }

        connect(d->configModule.get(), &KQuickAddons::ConfigModule::pagePushed, this, [this](QQuickItem *page) {
            QMetaObject::invokeMethod(d->pageRow, "push", Qt::DirectConnection, Q_ARG(QVariant, QVariant::fromValue(page)), Q_ARG(QVariant, QVariant()));
        });
        connect(d->configModule.get(), &KQuickAddons::ConfigModule::pageRemoved, this, [this]() {
            QMetaObject::invokeMethod(d->pageRow, "pop", Qt::DirectConnection, Q_ARG(QVariant, QVariant()));
        });
        connect(d->configModule.get(), &KQuickAddons::ConfigModule::currentIndexChanged, this, [this]() {
            d->pageRow->setProperty("currentIndex", d->configModule->currentIndex());
        });
        connect(d->configModule.get(),
                &KQuickAddons::ConfigModule::passiveNotificationRequested,
                this,
                [this](const QString &message, const QVariant &timeout, const QString &actionText, const QJSValue &callBack) {
                    d->rootPlaceHolder->metaObject()->invokeMethod(d->rootPlaceHolder,
                                                                   "showPassiveNotification",
                                                                   Q_ARG(QVariant, message),
                                                                   Q_ARG(QVariant, timeout),
                                                                   Q_ARG(QVariant, actionText),
                                                                   Q_ARG(QVariant, QVariant::fromValue(callBack)));
                });
        // New syntax cannot be used to connect to QML types
        connect(d->pageRow, SIGNAL(currentIndexChanged()), this, SLOT(syncCurrentIndex()));
    }

    layout->addWidget(d->quickWidget);
}

KCModuleQml::~KCModuleQml()
{
    delete d;
}

void KCModuleQml::load()
{
    d->configModule->load();
    setNeedsSave(d->configModule->representsDefaults());
}

void KCModuleQml::save()
{
    d->configModule->save();
    d->configModule->setNeedsSave(false);
}

void KCModuleQml::defaults()
{
    d->configModule->defaults();
}

#include "moc_kcmoduleqml_p.cpp"
