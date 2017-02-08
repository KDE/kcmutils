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
#include <KAboutData>
#include <KLocalizedString>
#include <KPackage/Package>
#include <KPackage/PackageLoader>

class KCModuleQmlPrivate
{
public:
    KCModuleQmlPrivate(KQuickAddons::ConfigModule *cm)
        : quickWindow(nullptr),
          configModule(cm)
    {
    }

    QQuickWindow *quickWindow;
    KQuickAddons::ConfigModule *configModule;
};

KCModuleQml::KCModuleQml(KQuickAddons::ConfigModule *configModule, QWidget* parent, const QVariantList& args)
    : KCModule(parent, args),
      d(new KCModuleQmlPrivate(configModule))
{

    connect(configModule, &KQuickAddons::ConfigModule::quickHelpChanged,
            this, &KCModuleQml::quickHelpChanged);
    //HACK:Here is important those two enums keep having the exact same values
    //but the kdeclarative one can't use the KCModule's enum
    setButtons((KCModule::Buttons)(int)d->configModule->buttons());
    connect(configModule, &KQuickAddons::ConfigModule::buttonsChanged, [=] {
        setButtons((KCModule::Buttons)(int)d->configModule->buttons());
    });

    if (d->configModule->needsSave()) {
        emit changed(true);
    }
    connect(configModule, &KQuickAddons::ConfigModule::needsSaveChanged, [=] {
        emit changed(d->configModule->needsSave());
    });

    setNeedsAuthorization(d->configModule->needsAuthorization());
    connect(configModule, &KQuickAddons::ConfigModule::needsAuthorizationChanged, [=] {
        setNeedsAuthorization(d->configModule->needsAuthorization());
    });

    setRootOnlyMessage(d->configModule->rootOnlyMessage());
    setUseRootOnlyMessage(d->configModule->useRootOnlyMessage());
    connect(configModule, &KQuickAddons::ConfigModule::rootOnlyMessageChanged, [=] {
        setRootOnlyMessage(d->configModule->rootOnlyMessage());
    });
    connect(configModule, &KQuickAddons::ConfigModule::useRootOnlyMessageChanged, [=] {
        setUseRootOnlyMessage(d->configModule->useRootOnlyMessage());
    });

    if (!d->configModule->authActionName().isEmpty()) {
        setAuthAction(KAuth::Action(d->configModule->authActionName()));
    }
    connect(configModule, &KQuickAddons::ConfigModule::authActionNameChanged, [=] {
        setAuthAction(d->configModule->authActionName());
    });
    setFocusPolicy(Qt::StrongFocus);
}

KCModuleQml::~KCModuleQml()
{
    delete d;
}

void KCModuleQml::showEvent(QShowEvent *event)
{
    if (d->quickWindow || !d->configModule->mainUi()) {
        KCModule::showEvent(event);
        return;
    }

    QVBoxLayout* layout = new QVBoxLayout(this);

    QQuickWidget *widget = new QQuickWidget(d->configModule->engine(), this);
    widget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    d->quickWindow = widget->quickWindow();
    d->quickWindow->setColor(QGuiApplication::palette().window().color());
    connect(qApp, &QGuiApplication::paletteChanged, d->quickWindow, [=]() {
        d->quickWindow->setColor(QGuiApplication::palette().window().color());
    });

    QQmlComponent *component = new QQmlComponent(d->configModule->engine(), this);
    component->setData(QByteArrayLiteral("import QtQuick 2.3\nItem{}"), QUrl());
    QObject *root = component->create();
    widget->setContent(QUrl(), component, root);

    d->configModule->mainUi()->setParentItem(widget->rootObject());

    //set anchors
    QQmlExpression expr(d->configModule->engine()->rootContext(), d->configModule->mainUi(), QStringLiteral("parent"));
    QQmlProperty prop(d->configModule->mainUi(), QStringLiteral("anchors.fill"));
    prop.write(expr.evaluate());

    layout->addWidget(widget);
    KCModule::showEvent(event);
}

void KCModuleQml::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    d->quickWindow->requestActivate();
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
