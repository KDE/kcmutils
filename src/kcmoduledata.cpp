/*
 * Copyright (c) 2020 Benjamin Port <benjamin.port@enioka.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kcmoduledata.h"

#include <QList>
#include <QDebug>
#include <QPointer>
#include <QVariantList>

#include <KConfigCore/KCoreConfigSkeleton>

class KCModuleDataPrivate
{
public:
    explicit KCModuleDataPrivate(KCModuleData *probe)
        : _q(probe)
    {
    }

    KCModuleData *_q;
    QList<QPointer<KCoreConfigSkeleton>> _skeletons;
};

KCModuleData::KCModuleData(QObject *parent, const QVariantList &)
    : QObject(parent), d(new KCModuleDataPrivate(this))
{
}

KCModuleData::~KCModuleData()
{
    delete d;
}

void KCModuleData::virtual_hook(int, void *)
{
}

void KCModuleData::registerSkeleton(KCoreConfigSkeleton *skeleton)
{
    if (!skeleton || d->_skeletons.contains(skeleton)) {
        return;
    }

    d->_skeletons.append(skeleton);
}

bool KCModuleData::isDefaults() const
{
    bool defaults = true;
    for (const auto &skeleton : qAsConst(d->_skeletons)) {
        defaults &= skeleton->isDefaults();
    }
    return defaults;
}

bool KCModuleData::revertToDefaults()
{
    for (const auto &skeleton : qAsConst(d->_skeletons)) {
        skeleton->useDefaults(true);
        skeleton->save();
    }
}

void KCModuleData::autoRegisterSkeletons()
{
    const auto skeletons = findChildren<KCoreConfigSkeleton*>();
    for (auto *skeleton : skeletons) {
        registerSkeleton(skeleton);
    }
}

bool KCModuleData::matchQuery(const QString query) const
{
    // Currently not implemented, here for future use case
    Q_UNUSED(query)
    return false;
}

#include "moc_kcmoduledata.cpp"
