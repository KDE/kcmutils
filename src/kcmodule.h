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
#if KCMUTILS_WITH_KAUTH
#include <KAuth/Action>
#endif

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
 * The base class for configuration modules.
 *
 * Configuration modules are realized as plugins that are loaded only when
 * needed.
 *
 * The module in principle is a simple widget displaying the
 * item to be changed. The module has a very small interface.
 *
 * All the necessary glue logic and the GUI bells and whistles
 * are provided by the control center and must not concern
 * the module author.
 *
 * To write a config module, you have to create a library
 * that contains a factory function like the following:
 *
 * \code
 * #include <KPluginFactory>
 *
 * K_PLUGIN_FACTORY(MyKCModuleFactory, registerPlugin<MyKCModule>() )
 * \endcode
 *
 * The constructor of the KCModule then looks like this:
 * \code
 * YourKCModule::YourKCModule( QWidget* parent )
 *   : KCModule( parent )
 * {
 *   KAboutData *about = new KAboutData(
 *     <kcm name>, i18n( "..." ),
 *     KDE_VERSION_STRING, QString(), KAboutLicense::GPL,
 *     i18n( "Copyright 2006 ..." ) );
 *   about->addAuthor( i18n(...) );
 *   setAboutData( about );
 *   .
 *   .
 *   .
 * }
 * \endcode
 *
 * If you want to make the KCModule available only conditionally (i.e. show in
 * the list of available modules only if some test succeeds) then you can use
 * Hidden in the .desktop file. An example:
 * \code
 * Hidden[$e]=$(if test -e /dev/js*; then echo "false"; else echo "true"; fi)
 * \endcode
 * The example executes the given code in a shell and uses the stdout output for
 * the Hidden value (so it's either Hidden=true or Hidden=false).
 *
 * See http://techbase.kde.org/Development/Tutorials/KCM_HowTo
 * for more detailed documentation.
 *
 * @author Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 * @since 6.0
 */
class KCMUTILS_EXPORT KCModule : public KAbstractConfigModule
{
    Q_OBJECT

public:
    /**
     * Base class for all KControlModules.
     *
     * @note do not emit changed signals here, since they are not yet connected
     *       to any slot.
     */
    explicit KCModule(QWidget *parent = nullptr, const KPluginMetaData &data = {}, const QVariantList &args = {});

    /**
     * Destroys the module.
     */
    ~KCModule() override;

    /**
     * @return a list of @ref KConfigDialogManager's in use, if any.
     */
    QList<KConfigDialogManager *> configs() const;

#if KCMUTILS_WITH_KAUTH
    /**
     * @brief Set if the module's save() method requires authorization to be executed
     *
     * It will still have to execute the action itself using the KAuth library, so
     * this method is not technically needed to perform the action, but
     * using this method will ensure that hosting
     * applications like System Settings or kcmshell behave correctly.
     *
     * @param action the action that will be used by this KCModule
     */
    void setAuthAction(const KAuth::Action &action);

    /**
     * Returns the action previously set with setAuthAction(). By default its an invalid action.
     *
     * @return The action that has to be authorized to execute the save() method.
     */
    KAuth::Action authAction() const;
#endif

    void load() override;
    void save() override;
    void defaults() override;

    /**
     * Get the associated widget that can be embedded
     *
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

    friend class KCModuleProxy;

protected Q_SLOTS:
    /**
     * A managed widget was changed, the widget settings and the current
     * settings are compared and a corresponding changed() signal is emitted
     */
    void widgetChanged();

#if KCMUTILS_WITH_KAUTH
    /**
     * The status of the auth action, if one, has changed
     */
    void authStatusChanged(KAuth::Action::AuthStatus status);
#endif

protected:
    /**
     * Returns the changed state of automatically managed widgets in this dialog
     */
    bool managedWidgetChangeState() const;

    /**
     * Returns the defaulted state of automatically managed widgets in this dialog
     *
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

private:
    std::unique_ptr<KCModulePrivate> const d;
};

#endif // KCMODULE_H
