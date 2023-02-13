/**
 * SPDX-FileCopyrightText: Year Author <author@domanin.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <KQuickManagedConfigModule>

class TimeSettings : public KQuickManagedConfigModule
{
    Q_OBJECT
public:
    TimeSettings(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
};
