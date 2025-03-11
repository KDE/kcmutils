/*
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCMODULEDATA_H
#define KCMODULEDATA_H

#include <QObject>
#include <QVariantList>
#include <kcmutilscore_export.h>
#include <memory>

class KCModuleDataPrivate;
class KCoreConfigSkeleton;

/*!
 * \class KCModuleData
 * \inmodule KCMUtilsCore
 * \brief A base class that offers information about a KCModule state.
 * \since 5.74
 */
class KCMUTILSCORE_EXPORT KCModuleData : public QObject
{
    Q_OBJECT
public:
    /*!
     *
     */
    explicit KCModuleData(QObject *parent = nullptr);
    ~KCModuleData() override;

    /*!
     * \brief Checks if the configuration is identical to the default one.
     *
     * Returns \c true if the module configuration is in the default state, \c false otherwise.
     */
    virtual bool isDefaults() const;

    /*!
     * \brief Reverts the module to default values and saves them.
     */
    virtual void revertToDefaults();

    /*!
     * \brief Returns whether this module matches a given \a query.
     *
     * The \a query is not expected to be a regex pattern but a full text search.
     */
    virtual bool matchesQuery(const QString &query) const;

Q_SIGNALS:
    /*!
     * \brief Emitted when KCModuleData is loaded.
     */
    void loaded();

    /*!
     * \internal
     * \brief Triggers the emit of \sa loaded() signal.
     *
     * This is the default behavior.
     *
     * To handle when loaded() is emitted in subclass,
     * disconnect this signal in the derived constructor.
     */
    void aboutToLoad(QPrivateSignal);

protected Q_SLOTS:
    /*!
     * \brief Allow to manually register a \a skeleton class.
     *
     * Used by a derived class when automatic discovery is not possible.
     */
    void registerSkeleton(KCoreConfigSkeleton *skeleton);

    /*!
     * \brief Automatically register child skeletons.
     *
     * Call it in your subclass constructor after skeleton creation
     */
    void autoRegisterSkeletons();

private:
    const std::unique_ptr<KCModuleDataPrivate> d;
    friend class KCModuleDataPrivate;
};

#endif
