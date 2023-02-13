/**
 * SPDX-FileCopyrightText: Year Author <author@domain.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "timesettings.h"

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(TimeSettings, "kcm_time.json")

TimeSettings::TimeSettings(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KQuickManagedConfigModule(parent, data, args)
{
    setButtons(Help | Apply | Default);
}

#include "timesettings.moc"
