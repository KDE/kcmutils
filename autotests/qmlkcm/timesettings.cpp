/*
 * SPDX-FileCopyrightText: Year Author <author@domain.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "timesettings.h"

#include <QtCore/qtsymbolmacros.h>

QT_DECLARE_EXTERN_SYMBOL_VOID(qml_register_types_org_kde_kcm_testqml2)

TimeSettings::TimeSettings(QObject *parent, const KPluginMetaData &data)
        : KQuickManagedConfigModule(parent, data)
{
    // QT_KEEP_SYMBOL(qml_register_types_org_kde_kcm_testqml2)
    qWarning() << Q_FUNC_INFO;
}
void TimeSettings::load()
{
    qWarning() << Q_FUNC_INFO;
}
