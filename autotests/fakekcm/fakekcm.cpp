/*
    This file is part of the KDE Frameworks
    SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KCModule>
#include <KPluginFactory>

class FakeKcm : public KCModule
{
};

K_PLUGIN_FACTORY(KCModuleFactory, registerPlugin<KCModule>();)

#include "fakekcm.moc"
