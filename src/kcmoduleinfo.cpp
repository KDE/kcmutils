/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
  Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>
  Copyright (c) 2003,2006 Matthias Kretz <kretz@kde.org>

  This file is part of the KDE project

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2, as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "kcmoduleinfo.h"

#include <QVariant>

#include <kdesktopfile.h>
#include <QDebug>

#include <KPluginInfo>
#include <klocalizedstring.h>

class Q_DECL_HIDDEN KCModuleInfo::Private
{
public:
    Private();
    Private(const KPluginInfo &);
    Private(const KService::Ptr &);

    QStringList keywords;
    QString     name, icon, lib, handle, fileName, doc, comment;
    bool        allLoaded = false;
    int         weight = 100;

    // For real C++ plugins
    KPluginInfo pluginInfo;

    // Can be a C++ plugin, or just a desktop file launching an executable (see autotest)
    KService::Ptr service;

    /**
     * Reads the service entries specific for KCModule from the desktop file.
     * The usual desktop entries are read in the Private ctor.
     */
    void loadAll();
};

KCModuleInfo::Private::Private()
{
}

KCModuleInfo::Private::Private(const KPluginInfo &pluginInfo)
    : allLoaded(false)
    , pluginInfo(pluginInfo)
{
    if (!pluginInfo.isValid()) {
        qWarning() << "Invalid plugin";
        return;
    }

    // set the modules simple attributes
    name = pluginInfo.name();
    comment = pluginInfo.comment();
    icon = pluginInfo.icon();
    fileName = pluginInfo.entryPath();
    lib = pluginInfo.libraryPath();
    keywords = pluginInfo.property(QStringLiteral("Keywords")).toStringList();
}

KCModuleInfo::Private::Private(const KService::Ptr &service)
    : allLoaded(false),
      pluginInfo(),
      service(service)
{
    if (!service) {
        return;
    }

    name = service->name();
    comment = service->comment();
    icon = service->icon();
    fileName = service->entryPath();
    lib = service->library();
    keywords = service->keywords();
}

KCModuleInfo::KCModuleInfo()
    : d(new Private)
{
}

KCModuleInfo::KCModuleInfo(const QString &desktopFile)
    : d(new Private(KService::serviceByStorageId(desktopFile)))
{
}

KCModuleInfo::KCModuleInfo(KService::Ptr service)
    : d(new Private(service))
{
}

KCModuleInfo::KCModuleInfo(const KPluginInfo &pluginInfo)
    : d(new Private(pluginInfo))
{
}

KCModuleInfo::KCModuleInfo(const KCModuleInfo &rhs)
    : d(new Private)
{
    (*this) = rhs;
}

KCModuleInfo &KCModuleInfo::operator=(const KCModuleInfo &rhs)
{
    *d = *(rhs.d);
    return *this;
}

bool KCModuleInfo::operator==(const KCModuleInfo &rhs) const
{
    return ((d->name == rhs.d->name) && (d->lib == rhs.d->lib) && (d->fileName == rhs.d->fileName));
}

bool KCModuleInfo::operator!=(const KCModuleInfo &rhs) const
{
    return ! operator==(rhs);
}

KCModuleInfo::~KCModuleInfo()
{
    delete d;
}

void KCModuleInfo::Private::loadAll()
{
    allLoaded = true;

    if (!pluginInfo.isValid() && !service) { /* We have a bogus service. All get functions will return empty/zero values */
        return;
    }

    if (service) {
        // get the documentation path
        doc = service->property(QStringLiteral("X-DocPath"), QVariant::String).toString();
        if (doc.isEmpty()) {
            doc = service->property(QStringLiteral("DocPath"), QVariant::String).toString();
        }

        // read weight
        QVariant tmp = service->property(QStringLiteral("X-KDE-Weight"), QVariant::Int);
        weight = tmp.isValid() ? tmp.toInt() : 100;

        // factory handle
        tmp = service->property(QStringLiteral("X-KDE-FactoryName"), QVariant::String);
        handle = tmp.isValid() ? tmp.toString() : lib;
    } else {
        // get the documentation path
        doc = pluginInfo.property(QStringLiteral("X-DocPath")).toString();
        if (doc.isEmpty()) {
            doc = pluginInfo.property(QStringLiteral("DocPath")).toString();
        }

        // read weight
        QVariant tmp = pluginInfo.property(QStringLiteral("X-KDE-Weight")).toInt();
        weight = tmp.isValid() ? tmp.toInt() : 100;

        // factory handle
        tmp = pluginInfo.property(QStringLiteral("X-KDE-FactoryName"));
        handle = tmp.isValid() ? tmp.toString() : lib;
    }
}

QString KCModuleInfo::fileName() const
{
    return d->fileName;
}

QStringList KCModuleInfo::keywords() const
{
    return d->keywords;
}

QString KCModuleInfo::moduleName() const
{
    return d->name;
}

KService::Ptr KCModuleInfo::service() const
{
    if (d->service) {
        return d->service;
    }
    if (!d->pluginInfo.isValid()) {
        return {};
    }
    return d->pluginInfo.service();
}

KPluginInfo KCModuleInfo::pluginInfo() const
{
    return d->pluginInfo;
}

QString KCModuleInfo::comment() const
{
    return d->comment;
}

QString KCModuleInfo::icon() const
{
    return d->icon;
}

QString KCModuleInfo::library() const
{
    return d->lib;
}

QString KCModuleInfo::docPath() const
{
    if (!d->allLoaded) {
        d->loadAll();
    }

    return d->doc;
}

QString KCModuleInfo::handle() const
{
    if (!d->allLoaded) {
        d->loadAll();
    }

    return d->handle;
}

int KCModuleInfo::weight() const
{
    if (!d->allLoaded) {
        d->loadAll();
    }

    return d->weight;
}

