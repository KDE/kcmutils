/*
    This file is part of the KDE Frameworks
    SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KPluginFactory>
#include <KPluginMetaData>
#include <kcmodule.h>

#include <QLabel>

class FakeKcm : public KCModule
{
public:
    FakeKcm(QWidget *parent, const KPluginMetaData &data, const QVariantList &args)
        : KCModule(parent, data, args)
    {
        auto label = new QLabel(widget());
        label->setText(QStringLiteral("Demo Text"));
    }
};

K_PLUGIN_CLASS_WITH_JSON(FakeKcm, "fakekcm.json")

#include "fakekcm.moc"
