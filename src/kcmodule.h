/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCMODULE_H
#define KCMODULE_H

#include "kcmutils_export.h"
#include <KAbstractConfigModule>
#include <KPluginMetaData>

#include <QVariant>
#include <QWidget>
#include <memory>

class KConfigDialogManager;
class KCoreConfigSkeleton;
class KConfigSkeleton;
class KCModulePrivate;

/**
 * @class KCModule kcmodule.h KCModule
 *
 * The base class for QWidgets configuration modules.
 * Configuration modules are loaded as plugins.
 *
 * The module in principle is a simple widget displaying the
 * item to be changed. The module has a very small interface.
 *
 * To write a config module, you have to create a library
 * that contains a factory class like the following:
 *
 * \code
 * #include <KPluginFactory>
 *
 * K_PLUGIN_CLASS_WITH_JSON(MyKCModule, "mykcmodule.json")
 * \endcode
 *
 * The constructor of the KCModule then looks like this:
 * \code
 * YourKCModule::YourKCModule(QWidget *parent, const KPluginMetaData &data, const QVariantList &args)
 *   : KCModule(parent, data, args)
 * {
 * // KCModule does not directly extend QWidget due to ambiguity with KAbstractConfigModule
 * // Because of this, you need to call widget() to get the parent widget
 * auto label = new QLabel(widget());
 * label->setText(QStringLiteral("Demo Text"));
 * }
 * \endcode
 *
 * This KCM can be loaded in a KCMultiDialog of kcmshell6
 *
 * @since 6.0
 */
class KCMUTILS_EXPORT KCModule : public KAbstractConfigModule
{
    Q_OBJECT

public:
    /**
     * Base class for all QWidgets configuration modules.
     *
     * @note do not emit changed signals here, since they are not yet connected
     *       to any slot.
     */
    explicit KCModule(QWidget *parent, const KPluginMetaData &data, const QVariantList &args);

    /**
     * Destroys the module.
     */
    ~KCModule() override;

    /**
     * @return a list of @ref KConfigDialogManager's in use, if any.
     */
    QList<KConfigDialogManager *> configs() const;

    void load() override;
    void save() override;
    void defaults() override;

    /**
     * Get the associated widget that can be embedded
     * The returned widget should be used as a parent for widgets you create
     */
    QWidget *widget() const;

protected:
    /**
     * Adds a KCoreConfigskeleton @p config to watch the widget @p widget
     *
     * This function is useful if you need to handle multiple configuration files.
     *
     * @return a pointer to the KCoreConfigDialogManager in use
     * @param config the KCoreConfigSkeleton to use
     * @param widget the widget to watch
     */
    KConfigDialogManager *addConfig(KCoreConfigSkeleton *config, QWidget *widget);

protected Q_SLOTS:
    /**
     * A managed widget was changed, the widget settings and the current
     * settings are compared and a corresponding needsSaveChanged() signal is emitted
     */
    void widgetChanged();

protected:
    /**
     * Returns the changed state of automatically managed widgets in this dialog
     */
    bool managedWidgetChangeState() const;

    /**
     * Returns the defaulted state of automatically managed widgets in this dialog
     */
    bool managedWidgetDefaultState() const;

    /**
     * Call this method when your manually managed widgets change state between
     * changed and not changed
     */
    void unmanagedWidgetChangeState(bool);

    /**
     * Call this method when your manually managed widgets change state between
     * defaulted and not defaulted
     */
    void unmanagedWidgetDefaultState(bool);

    /**
     * Utility overload to avoid having to take both parent and parentWidget
     * KCModuleLoader::loadModule enforces the parent to be a QWidget anyway
     */
    explicit KCModule(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
        : KCModule(qobject_cast<QWidget *>(parent), data, args)
    {
    }

private:
    std::unique_ptr<KCModulePrivate> const d;
};

#endif // KCMODULE_H
