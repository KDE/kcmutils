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

#include "kcmoduleqml.h"

#include <QDebug>

#include <QVBoxLayout>
#include <QtQml>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickItem>

#include <kdeclarative/kdeclarative.h>
#include <kdeclarative/qmlobject.h>
#include <quickaddons/configmodule.h>
#include <KAboutData>
#include <KLocalizedString>
#include <KPackage/Package>
#include <KPackage/PackageLoader>

class KCModuleQmlPrivate
{
public:
    KCModuleQmlPrivate(KDeclarative::ConfigModule *cm)
        : quickWindow(Q_NULLPTR),
          qmlObject(Q_NULLPTR),
          configModule(cm)
    {
    }

    QQuickWindow *quickWindow;
    KDeclarative::QmlObject *qmlObject;
    KDeclarative::ConfigModule *configModule;
};

KCModuleQml::KCModuleQml(KDeclarative::ConfigModule *configModule, QWidget* parent, const QVariantList& args)
    : KCModule(parent, args),
      d(new KCModuleQmlPrivate(configModule))
{
}

KCModuleQml::~KCModuleQml()
{
    delete d;
}

void KCModuleQml::showEvent(QShowEvent *event)
{
    if (d->quickWindow) {
        KCModule::showEvent(event);
        return;
    }

    QVBoxLayout* layout = new QVBoxLayout(this);

    d->quickWindow = new QQuickWindow();
    QWidget *widget = QWidget::createWindowContainer(d->quickWindow, this);

    //d->quickWindow->setResizeMode(QQuickWindow::SizeRootObjectToView);

    //d->quickWindow->setSource(QUrl::fromLocalFile(package.filePath("mainscript")));
    //setMinimumHeight(d->quickWindow->initialSize().height());

    d->configModule->mainUi()->setParentItem(d->quickWindow->contentItem());
    //set anchors
    QQmlExpression expr(d->configModule->engine()->rootContext(), d->configModule->mainUi(), QLatin1String("parent"));
    QQmlProperty prop(d->configModule->mainUi(), QLatin1String("anchors.fill"));
    prop.write(expr.evaluate());

    layout->addWidget(widget);
    KCModule::showEvent(event);
}

#include "moc_kcmoduleqml.cpp"
