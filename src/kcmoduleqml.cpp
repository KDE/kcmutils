/* This file is part of the KDE Project
   Copyright (c) 2014 Marco Martin <mart@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kcmoduleqml_p.h"

#include <QDebug>

#include <QVBoxLayout>
#include <QtQml>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickItem>
#include <QGuiApplication>
#include <QQuickWidget>

#include <kdeclarative/kdeclarative.h>
#include <kquickaddons/configmodule.h>
#include <kdeclarative/qmlobjectsharedengine.h>
#include <KAboutData>
#include <KLocalizedString>
#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <KPageWidget>

class KCModuleQmlPrivate
{
public:
    KCModuleQmlPrivate(KQuickAddons::ConfigModule *cm, KCModuleQml *q)
        : q(q),
          quickWindow(nullptr),
          configModule(cm)
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
    QQuickWindow *quickWindow;
    QQuickWidget *quickWidget;
    QQuickItem *rootPlaceHolder;
    QQuickItem *pageRow;
    KQuickAddons::ConfigModule *configModule;
    KDeclarative::QmlObjectSharedEngine *qmlObject;
};

KCModuleQml::KCModuleQml(KQuickAddons::ConfigModule *configModule, QWidget* parent, const QVariantList& args)
    : KCModule(parent, args),
      d(new KCModuleQmlPrivate(configModule, this))
{

    connect(configModule, &KQuickAddons::ConfigModule::quickHelpChanged,
            this, &KCModuleQml::quickHelpChanged);
    //HACK:Here is important those two enums keep having the exact same values
    //but the kdeclarative one can't use the KCModule's enum
    setButtons((KCModule::Buttons)(int)d->configModule->buttons());
    connect(configModule, &KQuickAddons::ConfigModule::buttonsChanged, this, [=] {
        setButtons((KCModule::Buttons)(int)d->configModule->buttons());
    });

    if (d->configModule->needsSave()) {
        emit changed(true);
    }
    connect(configModule, &KQuickAddons::ConfigModule::needsSaveChanged, this, [=] {
        emit changed(d->configModule->needsSave());
    });

    setNeedsAuthorization(d->configModule->needsAuthorization());
    connect(configModule, &KQuickAddons::ConfigModule::needsAuthorizationChanged, this, [=] {
        setNeedsAuthorization(d->configModule->needsAuthorization());
    });

    setRootOnlyMessage(d->configModule->rootOnlyMessage());
    setUseRootOnlyMessage(d->configModule->useRootOnlyMessage());
    connect(configModule, &KQuickAddons::ConfigModule::rootOnlyMessageChanged, this, [=] {
        setRootOnlyMessage(d->configModule->rootOnlyMessage());
    });
    connect(configModule, &KQuickAddons::ConfigModule::useRootOnlyMessageChanged, this, [=] {
        setUseRootOnlyMessage(d->configModule->useRootOnlyMessage());
    });

    if (!d->configModule->authActionName().isEmpty()) {
        setAuthAction(KAuth::Action(d->configModule->authActionName()));
    }
    connect(configModule, &KQuickAddons::ConfigModule::authActionNameChanged, this, [=] {
        setAuthAction(d->configModule->authActionName());
    });
    setAboutData(d->configModule->aboutData());
    setFocusPolicy(Qt::StrongFocus);



    //Build the UI
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    d->qmlObject = new KDeclarative::QmlObjectSharedEngine(this);
    d->quickWidget = new QQuickWidget(d->qmlObject->engine(), this);
    d->quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    d->quickWidget->setFocusPolicy(Qt::StrongFocus);
    d->quickWidget->installEventFilter(this);
    d->quickWindow = d->quickWidget->quickWindow();
    d->quickWindow->setColor(QGuiApplication::palette().window().color());
    connect(qApp, &QGuiApplication::paletteChanged, d->quickWindow, [=]() {
        d->quickWindow->setColor(QGuiApplication::palette().window().color());
    });

    QQmlComponent *component = new QQmlComponent(d->qmlObject->engine(), this);
    //this has activeFocusOnTab to notice when the navigation wraps
    //around, so when we need to go outside and inside
    //pushPage/popPage are needed as push of StackView can't be directly invoked from c++
    //because its parameters are QQmlV4Function which is not public
    //the managers of onEnter/ReturnPressed are a workaround of
    //Qt bug https://bugreports.qt.io/browse/QTBUG-70934
    component->setData(QByteArrayLiteral("import QtQuick 2.3\n"
        "import org.kde.kirigami 2.4 as Kirigami\n"
        "Kirigami.ApplicationItem{"
            //purely cosmetic space, those magic values are to align perfectly within Systemsettings
            "header: Item {height: Math.round(Kirigami.Units.gridUnit*0.25)}"
            // allow only one column by default
            "pageStack.defaultColumnWidth:width;"
            "pageStack.separatorVisible:false;"
            "pageStack.globalToolBar.style: pageStack.wideScreen ? Kirigami.ApplicationHeaderStyle.Titles : Kirigami.ApplicationHeaderStyle.Breadcrumb;"
            "pageStack.globalToolBar.showNavigationButtons:false;"
            "pageStack.globalToolBar.preferredHeight:Kirigami.Units.gridUnit*2;"
            "pageStack.globalToolBar.separatorVisible:false;"
            "activeFocusOnTab:true;"
            "Keys.onReturnPressed:{event.accepted=true}"
            "Keys.onEnterPressed:{event.accepted=true}"
        "}"), QUrl());

    d->rootPlaceHolder = qobject_cast<QQuickItem *>(component->create());
    d->quickWidget->setContent(QUrl(), component, d->rootPlaceHolder);

    d->pageRow = d->rootPlaceHolder->property("pageStack").value<QQuickItem *>();
    if (d->pageRow) {
        QMetaObject::invokeMethod(d->pageRow, "push", Qt::DirectConnection, Q_ARG(QVariant, QVariant::fromValue(d->configModule->mainUi())), Q_ARG(QVariant, QVariant()));

        connect(d->configModule, &KQuickAddons::ConfigModule::pagePushed, this, [this](QQuickItem *page) {
                QMetaObject::invokeMethod(d->pageRow, "push", Qt::DirectConnection, Q_ARG(QVariant, QVariant::fromValue(page)), Q_ARG(QVariant, QVariant()));
            }
        );
        connect(d->configModule, &KQuickAddons::ConfigModule::pageRemoved, this, [this]() {
                QMetaObject::invokeMethod(d->pageRow, "pop", Qt::DirectConnection,  Q_ARG(QVariant, QVariant()));
            }
        );
        connect(d->configModule, &KQuickAddons::ConfigModule::currentIndexChanged, this, [this]() {
            d->pageRow->setProperty("currentIndex", d->configModule->currentIndex());
            }
        );
        //New syntax cannot be used to connect to QML types
        connect(d->pageRow, SIGNAL(currentIndexChanged()), this, SLOT(syncCurrentIndex()));

        auto syncColumnWidth = [this](){
            d->pageRow->setProperty("defaultColumnWidth", d->configModule->columnWidth() > 0 ? d->configModule->columnWidth() : d->rootPlaceHolder->width());
        };
        syncColumnWidth();

        connect(d->configModule, &KQuickAddons::ConfigModule::columnWidthChanged,
                this, syncColumnWidth);
        connect(d->rootPlaceHolder, &QQuickItem::widthChanged,
                this, syncColumnWidth);

        //HACK: in order to work with old Systemsettings
        //search if we are in a KPageWidget, search ofr its page, and if it has
        //an header set, disable our own title
        //FIXME: eventually remove this hack
        QObject *candidate = this;
        while (candidate) {
            candidate = candidate->parent();
            KPageWidget *page = qobject_cast<KPageWidget *>(candidate);
            if (page && !page->currentPage()->header().isEmpty()) {
                QObject *globalToolBar = d->pageRow->property("globalToolBar").value<QObject *>();
                //5 is None
                globalToolBar->setProperty("style", 5);
            }
        }
    }
    
    layout->addWidget(d->quickWidget);
}

KCModuleQml::~KCModuleQml()
{
    delete d;
}

bool KCModuleQml::eventFilter(QObject* watched, QEvent* event)
{
    //FIXME: those are all workarounds around the QQuickWidget brokeness
    //BUG https://bugreports.qt.io/browse/QTBUG-64561
    if (watched == d->quickWidget && event->type() == QEvent::KeyPress) {
        //allow tab navigation inside the qquickwidget
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);

        QQuickItem *currentItem = d->quickWindow->activeFocusItem();
        if (!currentItem) {
            return KCModule::eventFilter(watched, event);
        }
        if (currentItem->scopedFocusItem()) {
            currentItem = currentItem->scopedFocusItem();
        }

        if (ke->key() == Qt::Key_Tab) {
            //nextItemInFocusChain will always return something, in the worst case will still be currentItem
            QQuickItem *nextItem = currentItem->nextItemInFocusChain(true);
            //when it arrives at the place holder item, go out of the qqw and
            //go to the other widgets around systemsettigns
            if (nextItem == d->rootPlaceHolder) {
                QWidget *w = d->quickWidget->nextInFocusChain();
                while (!w->isEnabled() || !(w->focusPolicy() & Qt::TabFocus)) {
                    w = w->nextInFocusChain();
                }
                w->setFocus(Qt::TabFocusReason);
            } else {
                nextItem->forceActiveFocus(Qt::TabFocusReason);
            }
            return true;
        } else if (ke->key() == Qt::Key_Backtab
                   || (ke->key() == Qt::Key_Tab && (ke->modifiers() & Qt::ShiftModifier))) {
            QQuickItem *nextItem = currentItem->nextItemInFocusChain(false);

            if (nextItem == d->rootPlaceHolder) {
                QWidget *w = d->quickWidget->previousInFocusChain();
                while (!w->isEnabled() || !(w->focusPolicy() & Qt::TabFocus)) {
                    w = w->previousInFocusChain();
                }
                w->setFocus(Qt::BacktabFocusReason);
            } else {
                nextItem->forceActiveFocus(Qt::BacktabFocusReason);
            }
            return true;
        }
    }
    return KCModule::eventFilter(watched, event);
}

void KCModuleQml::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    if (event->reason() == Qt::TabFocusReason) {
        d->rootPlaceHolder->nextItemInFocusChain(true)->forceActiveFocus(Qt::TabFocusReason);
    } else if (event->reason() == Qt::BacktabFocusReason) {
        d->rootPlaceHolder->nextItemInFocusChain(false)->forceActiveFocus(Qt::BacktabFocusReason);
    }
    
}

QSize KCModuleQml::sizeHint() const
{
    if (!d->configModule->mainUi()) {
        return QSize();
    }

    return QSize(d->configModule->mainUi()->implicitWidth(), d->configModule->mainUi()->implicitHeight());
}

QString KCModuleQml::quickHelp() const
{
    return d->configModule->quickHelp();
}

const KAboutData *KCModuleQml::aboutData() const
{
    return d->configModule->aboutData();
}

void KCModuleQml::load()
{
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

#include "moc_kcmoduleqml_p.cpp"
