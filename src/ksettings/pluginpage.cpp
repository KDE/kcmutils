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

    KPluginSelector *m_pluginSelector = nullptr;
    void _k_reparseConfiguration(const QByteArray &a);
};

PluginPage::PluginPage(const KAboutData *aboutData, QWidget *parent, const QVariantList &args)
    : KCModule(aboutData, parent, args)
    , d_ptr(new PluginPagePrivate)
{
    Q_D(PluginPage);
    d->m_pluginSelector = new KPluginSelector(this);
    connect(d->m_pluginSelector, &KPluginSelector::changed, this, &KCModule::changed);
    connect(d->m_pluginSelector, &KPluginSelector::configCommitted, this, [d](const QByteArray &componentName) {
        d->_k_reparseConfiguration(componentName);
    });
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
    return d_ptr->m_pluginSelector;
}

void PluginPage::load()
{
    d_ptr->m_pluginSelector->load();
}

void PluginPage::save()
{
    d_ptr->m_pluginSelector->save();
}

void PluginPage::defaults()
{
    d_ptr->m_pluginSelector->defaults();
}

#endif

} // namespace

#include "moc_pluginpage.cpp"
