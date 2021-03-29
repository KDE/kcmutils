/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2006-2007 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ksettingswidgetadaptor.h"
#include <QGuiApplication>
#include <QString>

#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 82)
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
#endif
