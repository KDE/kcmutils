/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2006-2007 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSETTINGSWIDGETADAPTOR_H
#define KSETTINGSWIDGETADAPTOR_H

#include <QObject>
class QString;

/*
 * Simple D-Bus object to return the KGlobal::caption()
 */
class KSettingsWidgetAdaptor : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.internal.KSettingsWidget")
public:
    KSettingsWidgetAdaptor(QObject *parent);

public Q_SLOTS:
    QString applicationName();
};

#endif // KSETTINGSWIDGETADAPTOR_H
