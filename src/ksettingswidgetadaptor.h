/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2006-2007 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSETTINGSWIDGETADAPTOR_H
#define KSETTINGSWIDGETADAPTOR_H

#include "kcmutils_export.h"
#include <QObject>
class QString;

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 82)
/*
 * Simple D-Bus object to return the KGlobal::caption()
 * @deprecated Since 5.82, deprecated for lack of usage
 */
class KSettingsWidgetAdaptor : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.internal.KSettingsWidget")
public:
    KCMUTILS_DEPRECATED_VERSION(5, 82, "deprecated for lack of usage")
    KSettingsWidgetAdaptor(QObject *parent);

public Q_SLOTS:
    QString applicationName();
};

#endif
#endif // KSETTINGSWIDGETADAPTOR_H
