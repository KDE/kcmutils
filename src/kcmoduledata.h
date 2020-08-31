/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
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

    /**
    * Revert module to default value and save them
    */
    virtual void revertToDefaults();

    /**
     * return true if this module match a given query, used by module search engine
     * @param query
     * @return true if this module match a given query, used by module search engine
     */
    virtual bool matchQuery(const QString query) const;

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
