/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCMODULEQML_H
#define KCMODULEQML_H

#include <kdeclarative/kdeclarative_export.h>

#include <KCModule>
#include <memory>

class QQuickItem;
class KCModuleQmlPrivate;

namespace KQuickAddons
{
class ConfigModule;
}

class KCModuleQml : public KCModule
{
    Q_OBJECT

public:
    KCModuleQml(std::unique_ptr<KQuickAddons::ConfigModule> configModule, QWidget *parent, const QVariantList &args);
    ~KCModuleQml() override;

    QString quickHelp() const override;
#if KDECLARATIVE_BUILD_DEPRECATED_SINCE(5, 90)
    const KAboutData *aboutData() const override;
#endif

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

protected:
    void focusInEvent(QFocusEvent *event) override;
    QSize sizeHint() const override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    KCModuleQmlPrivate *const d;

    Q_PRIVATE_SLOT(d, void syncCurrentIndex())
};

#endif
