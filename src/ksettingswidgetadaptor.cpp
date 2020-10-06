/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2006-2007 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ksettingswidgetadaptor.h"
#include <QGuiApplication>
#include <QString>

#include "kcmoduleproxy.h"

KSettingsWidgetAdaptor::KSettingsWidgetAdaptor(QObject *parent)
    : QObject(parent)
{
}

QString KSettingsWidgetAdaptor::applicationName()
{
    const QString displayName = QGuiApplication::applicationDisplayName();
    if (!displayName.isEmpty()) {
        return displayName;
    }
    return QCoreApplication::applicationName();
}

void KSettingsWidgetAdaptor::updateArguments(const QStringList &args, const QVariantList &platform_data)
{
    static_cast<KCModuleProxy *>(parent())->updateArguments(args, platform_data);
}
