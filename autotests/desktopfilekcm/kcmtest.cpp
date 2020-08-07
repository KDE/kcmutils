/*
    SPDX-FileCopyrightText: 2020 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kcmtest.h"
#include <KPluginFactory>

KCMTest::KCMTest(QObject *parent, const QVariantList &args)
    : QObject(parent)
{
    Q_UNUSED(args)
}

K_PLUGIN_FACTORY_WITH_JSON(kcmtestfactory, "kcmtest.json", registerPlugin<KCMTest>();)

#include "kcmtest.moc"
