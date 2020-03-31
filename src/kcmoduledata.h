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

#ifndef KCMODULEDATA_H
#define KCMODULEDATA_H

#include <QObject>
#include <kcmutils_export.h>
#include <KConfigCore/KCoreConfigSkeleton>

class KCModuleDataPrivate;

/**
 * @short A base class that offers information about a KCModule state
 *
 * @author Benjamin Port <benjamin.port@enioka.com>
 *
 * @since 5.74
 */
class KCMUTILS_EXPORT KCModuleData : public QObject
{
    Q_OBJECT
public:
    explicit KCModuleData(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~KCModuleData() override;

    /**
    * return false if configuration is not the default one else true
    *
    * @return a boolean representing if module configuration is in default state
    */
    virtual bool isDefaults() const;

protected Q_SLOTS:
    /**
     * Allow to register manually skeleton class.
     * Used by derived class when automatic discovery is not possible.
     */
    void registerSkeleton(KCoreConfigSkeleton *skeleton);

    /**
     * Automatically register child skeletons
     * Call it in your subclass constructor after skeleton creation
     */
    void autoRegisterSkeletons();

protected:
    virtual void virtual_hook(int id, void *data);

private:
    KCModuleDataPrivate *const d;
    friend class KCModuleDataPrivate;
};

#endif
