/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCMODULEQML_H
#define KCMODULEQML_H

#include "kcmodule.h"

#include <memory>

class QQuickItem;
class QQmlEngine;
class KCModuleQmlPrivate;

namespace KQuickAddons
{
class ConfigModule;
}

class KCModuleQml : public KCModule
{
    Q_OBJECT

public:
    KCModuleQml(std::shared_ptr<QQmlEngine> engine, KQuickAddons::ConfigModule *configModule, QWidget *parent, const QVariantList &args);
    ~KCModuleQml() override;

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

private:
    friend class QmlConfigModuleWidget;
    KCModuleQmlPrivate *const d;

    Q_PRIVATE_SLOT(d, void syncCurrentIndex())
};

#endif
