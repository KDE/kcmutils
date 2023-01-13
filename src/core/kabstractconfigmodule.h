/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KABSTRACTCONFIGMODULE_H
#define KABSTRACTCONFIGMODULE_H

#include "kcmutilscore_export.h"

#include <QObject>

#include <memory>

class KPluginMetaData;
class KAbstractConfigModulePrivate;

/**
 * Base class for QML and QWidgets config modules.
 *
 * @author Alexander Lohnau
 * @since 6.0
 */
class KCMUTILSCORE_EXPORT KAbstractConfigModule : public QObject
{
    Q_OBJECT

    Q_PROPERTY(KAbstractConfigModule::Buttons buttons READ buttons WRITE setButtons NOTIFY buttonsChanged)
    Q_PROPERTY(int defaultsIndicatorsVisible READ defaultsIndicatorsVisible WRITE setDefaultsIndicatorsVisible NOTIFY defaultsIndicatorsVisibleChanged)
    Q_PROPERTY(bool needsAuthorization READ needsAuthorization WRITE setNeedsAuthorization NOTIFY needsAuthorizationChanged)
    Q_PROPERTY(bool representsDefaults READ representsDefaults WRITE setRepresentsDefaults NOTIFY representsDefaultsChanged)
    Q_PROPERTY(bool needsSave READ needsSave WRITE setNeedsSave NOTIFY needsSaveChanged)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QString quickHelp READ quickHelp WRITE setQuickHelp NOTIFY quickHelpChanged)
    Q_PROPERTY(QString rootOnlyMessage READ rootOnlyMessage WRITE setRootOnlyMessage NOTIFY rootOnlyMessageChanged)
    Q_PROPERTY(bool useRootOnlyMessage READ useRootOnlyMessage WRITE setUseRootOnlyMessage NOTIFY useRootOnlyMessageChanged)
public:
    /**
     * An enumeration type for the buttons used by this module.
     * You should only use Help, Default and Apply. The rest is obsolete.
     * NoAdditionalButton can be used when we do not want have other button that Ok Cancel
     *
     * @see ConfigModule::buttons @see ConfigModule::setButtons
     */
    enum Button {
        NoAdditionalButton = 0,
        Help = 1,
        Default = 2,
        Apply = 4,
        Export = 8,
    };
    Q_ENUM(Button)
    Q_DECLARE_FLAGS(Buttons, Button)
    Q_FLAG(Buttons)

    explicit KAbstractConfigModule(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);

    ~KAbstractConfigModule() override;

    /**
     * @brief Set if the module's save() method requires authorization to be executed
     *
     * It will still have to execute the action itself using the KAuth library, so
     * this method is not technically needed to perform the action, but
     * using this method will ensure that hosting
     * applications like System Settings or kcmshell behave correctly.
     *
     * @param action the action that will be used by this ConfigModule
     */
    void setAuthActionName(const QString &action);

    /**
     * Returns the action previously set with setAuthActionName(). By default its an invalid action.
     *
     * @return The action that has to be authorized to execute the save() method.
     */
    QString authActionName() const;

    /**
     * The auth action name has changed: this signal will relay it to the actual KCM
     */
    Q_SIGNAL void authActionNameChanged();

    /**
     * Sets the quick help.
     */
    void setQuickHelp(const QString &help);

    /**
     * Return a quick-help text.
     *
     * This method is called when the module is docked.
     * The quick-help text should contain a short description of the module and
     * links to the module's help files. You can use HTML formatting tags in the text.
     *
     * @note make sure the quick help text gets translated (use i18n()).
     */
    QString quickHelp() const;

    /**
     * Indicate that the module's quickhelp has changed.
     *
     * Emit this signal whenever the module's quickhelp changes.
     * Modules implemented as tabbed dialogs might want to implement
     * per-tab quickhelp for example.
     *
     */
    Q_SIGNAL void quickHelpChanged();

    /**
     * Set this property to true when the user changes something in the module,
     * signaling that a save (such as user pressing Ok or Apply) is needed.
     */
    void setNeedsSave(bool needs);

    /**
     * True when the module has something changed and needs save.
     */
    bool needsSave() const;

    /**
     * Indicate that the state of the modules contents has changed.
     *
     * This signal is emitted whenever the state of the configuration
     * shown in the module changes. It allows the module container to
     * keep track of unsaved changes.
     */
    Q_SIGNAL void needsSaveChanged();

    /**
     * Set this property to true when the user sets the state of the module
     * to the default settings (e.g. clicking Defaults would do nothing).
     */
    void setRepresentsDefaults(bool defaults);

    /**
     * True when the module state represents the default settings.
     */
    bool representsDefaults() const;

    /**
     * Indicate that the state of the modules contents has changed
     * in a way that it might represents the defaults settings, or
     * stopped representing them.
     */
    Q_SIGNAL void representsDefaultsChanged();

    /**
     * Sets the buttons to display.
     *
     * Help: shows a "Help" button.
     *
     * Default: shows a "Use Defaults" button.
     *
     * Apply: in kcontrol this will show an "Apply" and "Reset" button,
     *        in kcmshell this will show an "Ok", "Apply" and "Cancel" button.
     *
     * If Apply is not specified, kcmshell will show a "Close" button.
     *
     * @see ConfigModule::buttons
     */
    void setButtons(const Buttons btn);

    /**
     * Indicate which buttons will be used.
     *
     * The return value is a value or'ed together from
     * the Button enumeration type.
     *
     * @see ConfigModule::setButtons
     */
    Buttons buttons() const;

    /**
     * Buttons to display changed.
     */
    Q_SIGNAL void buttonsChanged();

    /**
     * Sets the RootOnly message.
     *
     * This message will be shown at the top of the module if useRootOnlyMessage is
     * set. If no message is set, a default one will be used.
     *
     * @see ConfigModule::rootOnlyMessage
     */
    void setRootOnlyMessage(const QString &message);

    /**
     * Get the RootOnly message for this module.
     *
     * When the module must be run as root, or acts differently
     * for root and a normal user, it is sometimes useful to
     * customize the message that appears at the top of the module
     * when used as a normal user. This function returns this
     * customized message. If none has been set, a default message
     * will be used.
     *
     * @see ConfigModule::setRootOnlyMessage
     */
    QString rootOnlyMessage() const;

    /**
     * Indicate that the module's root message has changed.
     *
     * Emits this signal whenever the module's root message changes.
     *
     */
    Q_SIGNAL void rootOnlyMessageChanged();

    /**
     * Change whether or not the RootOnly message should be shown.
     *
     * Following the value of @p on, the RootOnly message will be
     * shown or not.
     *
     * @see ConfigModule::useRootOnlyMessage
     */
    void setUseRootOnlyMessage(bool on);

    /**
     * Tell if KControl should show a RootOnly message when run as
     * a normal user.
     *
     * In some cases, the module don't want a RootOnly message to
     * appear (for example if it has already one). This function
     * tells KControl if a RootOnly message should be shown
     *
     * @see ConfigModule::setUseRootOnlyMessage
     */
    bool useRootOnlyMessage() const;

    /**
     * Emits this signal whenever the root only message gets used or discarded.
     */
    Q_SIGNAL void useRootOnlyMessageChanged();

    /**
     * @brief Set if the module's save() method requires authorization to be executed.
     *
     * The module can set this property to @c true if it requires authorization.
     * It will still have to execute the action itself using the KAuth library, so
     * this method is not technically needed to perform the action, but
     * using this and/or the setAuthActionName() method will ensure that hosting
     * applications like System Settings or kcmshell behave correctly.
     *
     * Called with @c true, this method will set the action to  "org.kde.kcontrol.name.save" where
     * "name" is aboutData()->appName() return value. This default action won't be set if
     * the aboutData() object is not valid.
     *
     * Note that called with @c false, this method will reset the action name set with setAuthActionName().
     *
     * @param needsAuth Tells if the module's save() method requires authorization to be executed.
     */
    void setNeedsAuthorization(bool needsAuth);

    /**
     * Returns the value previously set with setNeedsAuthorization() or setAuthActionName(). By default it's @c false.
     *
     * @return @c true if the module's save() method requires authorization, @c false otherwise
     */
    bool needsAuthorization() const;

    /**
     * Emits this signal whenever the need for root authorization to save changes.
     */
    Q_SIGNAL void needsAuthorizationChanged();

    /**
     * @returns the name of the config module
     */
    QString name() const;

    /**
     * @returns the description of the config module
     */
    QString description() const;

    /**
     * Change defaultness indicator visibility
     * @param visible
     */
    void setDefaultsIndicatorsVisible(bool visible);

    /**
     * @returns defaultness indicator visibility
     */
    bool defaultsIndicatorsVisible() const;

    /**
     * Emitted when kcm need to display indicators for field with non default value
     */
    Q_SIGNAL void defaultsIndicatorsVisibleChanged();

    /**
     * Returns the metaData that was used when instantiating the plugin
     */
    KPluginMetaData metaData() const;

    /**
     * Load the configuration data into the module.
     *
     * The load method sets the user interface elements of the
     * module to reflect the current settings stored in the
     * configuration files.
     *
     * This method is invoked whenever the module should read its configuration
     * (most of the times from a config file) and update the user interface.
     * This happens when the user clicks the "Reset" button in the control
     * center, to undo all of his changes and restore the currently valid
     * settings. It is also called right after construction.
     */
    virtual void load();

    /**
     * Save the configuration data.
     *
     * The save method stores the config information as shown
     * in the user interface in the config files.
     *
     * If necessary, this method also updates the running system,
     * e.g. by restarting applications. This normally does not apply for
     * KSettings::Dialog modules where the updating is taken care of by
     * KSettings::Dispatcher.
     *
     * save is called when the user clicks "Apply" or "Ok".
     *
     */
    virtual void save();

    /**
     * Sets the configuration to sensible default values.
     *
     * This method is called when the user clicks the "Default"
     * button. It should set the display to useful values.
     */
    virtual void defaults();

private:
    const std::unique_ptr<KAbstractConfigModulePrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KAbstractConfigModule::Buttons)

#endif // KABSTRACTCONFIGMODULE_H
