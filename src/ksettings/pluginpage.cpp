/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ksettings/pluginpage.h"
#include "kpluginselector.h"
#include "ksettings/dispatcher.h"
#include <KAboutData>

namespace KSettings
{
#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 76)

class PluginPagePrivate
{
public:
    PluginPagePrivate()
    {
    }

    KPluginSelector *selwid = nullptr;
    void _k_reparseConfiguration(const QByteArray &a);
};

PluginPage::PluginPage(const KAboutData *aboutData, QWidget *parent, const QVariantList &args)
    : KCModule(aboutData, parent, args)
    , d_ptr(new PluginPagePrivate)
{
    Q_D(PluginPage);
    d->selwid = new KPluginSelector(this);
    connect(d->selwid, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
    connect(d->selwid, SIGNAL(configCommitted(QByteArray)), this, SLOT(_k_reparseConfiguration(QByteArray)));
}

void PluginPagePrivate::_k_reparseConfiguration(const QByteArray &a)
{
    Dispatcher::reparseConfiguration(QString::fromLatin1(a));
}

PluginPage::~PluginPage()
{
    delete d_ptr;
}

KPluginSelector *PluginPage::pluginSelector()
{
    return d_ptr->selwid;
}

void PluginPage::load()
{
    //     d_ptr->selwid->load();
}

void PluginPage::save()
{
    d_ptr->selwid->save();
}

void PluginPage::defaults()
{
    d_ptr->selwid->defaults();
}

#endif

} // namespace

#include "moc_pluginpage.cpp"
