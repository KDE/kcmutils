/*
 * SPDX-FileCopyrightText: Year Author <author@domain.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KQuickManagedConfigModule>

class TimeSettings : public KQuickManagedConfigModule
{
    Q_OBJECT
    QML_ELEMENT
public:
    TimeSettings(QObject *parent = nullptr, const KPluginMetaData &data = {});
    void load() override;
};
