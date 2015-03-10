/*
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

#ifndef KCMODULEQML_H
#define KCMODULEQML_H

#include <kcmodule.h>

class QQuickItem;
class KCModuleQmlPrivate;

namespace KDeclarative {
    class ConfigModule;
}

class KCModuleQml : public KCModule
{
    Q_OBJECT

public:
    KCModuleQml(KDeclarative::ConfigModule *configModule, QWidget* parent, const QVariantList& args);
    ~KCModuleQml();

    virtual QString quickHelp() const;
    virtual const KAboutData *aboutData() const;

public Q_SLOTS:
    void load();
    void save();
    void defaults();

protected:
    virtual void showEvent(QShowEvent *event);
    QSize sizeHint() const;

private:
    KCModuleQmlPrivate *const d;
};

#endif
