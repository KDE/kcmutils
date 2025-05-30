/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcmoduleqml_p.h"

#include <QQuickItem>
#include <QQuickWidget>
#include <QQuickWindow>
#include <QTimer>
#include <QVBoxLayout>

#include <KAboutData>
#include <KLocalizedContext>
#include <KPageWidget>
#include <QQmlEngine>

#include "quick/kquickconfigmodule.h"

#include <kcmutils_debug.h>
#include <qquickitem.h>

class QmlConfigModuleWidget;
class KCModuleQmlPrivate
{
public:
    KCModuleQmlPrivate(KQuickConfigModule *cm, KCModuleQml *qq)
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
    KQuickConfigModule *configModule;
    QmlConfigModuleWidget *widget = nullptr;
};

class QmlConfigModuleWidget : public QWidget
{
    Q_OBJECT
public:
    QmlConfigModuleWidget(KCModuleQml *module, QWidget *parent)
        : QWidget(parent)
        , m_module(module)
    {
        setFocusPolicy(Qt::StrongFocus);
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
        // Everything would work mosty without manual intervention, but as of Qt 6.8
        // things require special attention so that they work correctly with orca.
        // The timing between the focusproxied QQuickWidget receiving focus and the
        // focused qml Item being registered as focused is off and screen readers get
        // confused. Instead, put initial focus on the root element and switch with a timer
        // so the qml focuschange happens while the qquickwidget has focus. This
        // requires activeFocusOnTab on the rootPlaceHolder to work, and that makes other things
        // a bit messier than they would otherwise need to be.
        if (event->type() == QEvent::FocusIn && watched == m_module->d->rootPlaceHolder) {
            auto focusEvent = static_cast<QFocusEvent *>(event);
            if (focusEvent->reason() == Qt::TabFocusReason) {
                m_module->d->rootPlaceHolder->forceActiveFocus(Qt::OtherFocusReason);
                QTimer::singleShot(0, this, [this] {
                    QQuickItem *nextItem = m_module->d->rootPlaceHolder->nextItemInFocusChain(true);
                    if (nextItem) {
                        nextItem->forceActiveFocus(Qt::TabFocusReason);
                    }
                });
                return true;
            } else if (focusEvent->reason() == Qt::BacktabFocusReason) {
                // this can either happen from backtabbing in qml or from backtabbing
                // from qwidgets past the focusproxy (e.g. from the kcm buttons).
                if (!m_module->d->rootPlaceHolder->hasActiveFocus()) {
                    // we're in widgets, enter qml from reverse in the focus chain in the same way as above
                    QTimer::singleShot(0, this, [this] {
                        QQuickItem *nextItem = m_module->d->rootPlaceHolder->nextItemInFocusChain(false);
                        if (nextItem) {
                            nextItem->forceActiveFocus(Qt::TabFocusReason);
                        }
                    });
                    return true;
                }
                // we're coming from qml, so we focus the widget outside. This also needs singleShot;
                // if we do it immediately, focus cycles backward along the qml focus chain instead.
                // Without activeFocusOnTab on the rootPlaceHolder we could just return false and
                // Qt would handle everything by itself
                QTimer::singleShot(0, this, [this] {
                    focusNextPrevChild(false);
                });
                return true;
            }
        }
        return QWidget::eventFilter(watched, event);
    }

private:
    KCModuleQml *m_module;
};

KCModuleQml::KCModuleQml(KQuickConfigModule *configModule, QWidget *parent)
    : KCModule(parent, configModule->metaData())
    , d(new KCModuleQmlPrivate(configModule, this))
{
    d->widget = new QmlConfigModuleWidget(this, parent);
    setButtons(d->configModule->buttons());
    connect(d->configModule, &KQuickConfigModule::buttonsChanged, d->configModule, [this] {
        setButtons(d->configModule->buttons());
    });

    setNeedsSave(d->configModule->needsSave());
    connect(d->configModule, &KQuickConfigModule::needsSaveChanged, this, [this] {
        setNeedsSave(d->configModule->needsSave());
    });

    setRepresentsDefaults(d->configModule->representsDefaults());
    connect(d->configModule, &KQuickConfigModule::representsDefaultsChanged, this, [this] {
        setRepresentsDefaults(d->configModule->representsDefaults());
    });

    setAuthActionName(d->configModule->authActionName());
    connect(d->configModule, &KQuickConfigModule::authActionNameChanged, this, [this] {
        setAuthActionName(d->configModule->authActionName());
    });

    connect(this, &KCModule::defaultsIndicatorsVisibleChanged, d->configModule, [this] {
        d->configModule->setDefaultsIndicatorsVisible(defaultsIndicatorsVisible());
    });

    connect(this, &KAbstractConfigModule::activationRequested, d->configModule, &KQuickConfigModule::activationRequested);

    // Build the UI
    QVBoxLayout *layout = new QVBoxLayout(d->widget);
    layout->setContentsMargins(0, 0, 0, 0);

    d->quickWidget = new QQuickWidget(d->configModule->engine().get(), d->widget);
    d->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    d->quickWidget->setAttribute(Qt::WA_AlwaysStackOnTop, true);
    d->quickWidget->setAttribute(Qt::WA_NoMousePropagation, true); // Workaround for QTBUG-109861 to fix drag everywhere
    d->quickWindow = d->quickWidget->quickWindow();
    d->quickWindow->setColor(Qt::transparent);
    d->widget->setFocusProxy(d->quickWidget);

    QQmlComponent *component = new QQmlComponent(d->configModule->engine().get(), this);
    // activeFocusOnTab is required to have screen readers not get confused
    // pushPage/popPage are needed as push of StackView can't be directly invoked from c++
    // because its parameters are QQmlV4Function which is not public.
    // The managers of onEnter/ReturnPressed are a workaround of
    // Qt bug https://bugreports.qt.io/browse/QTBUG-70934
    // clang-format off
    // TODO: move this in an instantiable component which would be used by the qml-only version as well
    component->setData(QByteArrayLiteral(R"(
import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami 2 as Kirigami
import org.kde.kcmutils as KCMUtils

Kirigami.ApplicationItem {
    // force it to *never* try to resize itself
    width: Window.width

    implicitWidth: Math.max(pageStack.implicitWidth, Kirigami.Units.gridUnit * 36)
    implicitHeight: Math.max(pageStack.implicitHeight, Kirigami.Units.gridUnit * 20)

    activeFocusOnTab: true

    property KCMUtils.ConfigModule kcm

    QQC2.ToolButton {
        id: toolButton
        visible: false
        icon.name: "go-previous"
    }

    pageStack.separatorVisible: pageStack.depth > 0 && (pageStack.items[0].sidebarMode ?? false)
    pageStack.globalToolBar.preferredHeight: toolButton.implicitHeight + Kirigami.Units.smallSpacing * 2
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.ToolBar
    pageStack.globalToolBar.showNavigationButtons: pageStack.currentIndex > 0 ? Kirigami.ApplicationHeaderStyle.ShowBackButton : Kirigami.ApplicationHeaderStyle.NoNavigationButtons

    pageStack.columnView.columnResizeMode: pageStack.items.length > 0 && (pageStack.items[0].Kirigami.ColumnView.fillWidth || pageStack.items.filter(item => item.visible).length === 1)
        ? Kirigami.ColumnView.SingleColumn
        : Kirigami.ColumnView.FixedColumns

    pageStack.defaultColumnWidth: kcm && kcm.columnWidth > 0 ? kcm.columnWidth : Kirigami.Units.gridUnit * 15

    footer: null
    Keys.onReturnPressed: event => {
        event.accepted = true
    }
    Keys.onEnterPressed: event => {
        event.accepted = true
    }

    Window.onWindowChanged: {
        if (Window.window) {
            Window.window.LayoutMirroring.enabled = Qt.binding(() => Qt.application.layoutDirection === Qt.RightToLeft)
            Window.window.LayoutMirroring.childrenInherit = true
        }
    }
}
    )"), QUrl(QStringLiteral("kcmutils/kcmmoduleqml.cpp")));
    // clang-format on

    d->rootPlaceHolder = qobject_cast<QQuickItem *>(component->create());
    if (!d->rootPlaceHolder) {
        qCCritical(KCMUTILS_LOG) << component->errors();
        qFatal("Failed to initialize KCModuleQML");
    }
    d->rootPlaceHolder->setProperty("kcm", QVariant::fromValue(d->configModule));
    d->rootPlaceHolder->installEventFilter(d->widget);
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
            if (d->configModule->mainUi()->property("sidebarMode").toBool()) {
                d->pageRow->setProperty("currentIndex", 0);
                d->configModule->setCurrentIndex(0);
            }
        }

        connect(d->configModule, &KQuickConfigModule::pagePushed, this, [this](QQuickItem *page) {
            QMetaObject::invokeMethod(d->pageRow, "push", Qt::DirectConnection, Q_ARG(QVariant, QVariant::fromValue(page)), Q_ARG(QVariant, QVariant()));
        });
        connect(d->configModule, &KQuickConfigModule::pageRemoved, this, [this]() {
            QMetaObject::invokeMethod(d->pageRow, "pop", Qt::DirectConnection, Q_ARG(QVariant, QVariant()));
        });
        connect(d->configModule, &KQuickConfigModule::currentIndexChanged, this, [this]() {
            d->pageRow->setProperty("currentIndex", d->configModule->currentIndex());
        });
        // New syntax cannot be used to connect to QML types
        connect(d->pageRow, SIGNAL(currentIndexChanged()), this, SLOT(syncCurrentIndex()));
    }

    layout->addWidget(d->quickWidget);
}

KCModuleQml::~KCModuleQml() = default;

void KCModuleQml::load()
{
    KCModule::load(); // calls setNeedsSave(false)
    d->configModule->load();
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

QWidget *KCModuleQml::widget()
{
    return d->widget;
}

#include "kcmoduleqml.moc"
#include "moc_kcmoduleqml_p.cpp"
