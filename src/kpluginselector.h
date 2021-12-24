/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007, 2006 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KPLUGINSELECTOR_H
#define KPLUGINSELECTOR_H

#include <kcmutils_export.h>

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 90)

#include <QWidget>

#include <QList>

#include <KSharedConfig>

class KPluginInfo;
class QPushButton;

/**
 * @short A widget to select what plugins to load and configure the plugins.
 *
 * It shows the list of available plugins
 *
 * Since the user needs a way to know what a specific plugin does every plugin
 * should install a desktop file containing a name, comment and category field.
 * The category is useful for applications that can use different kinds of
 * plugins like a playlist, skin or visualization
 *
 * The location of these desktop files is the
 * share/apps/&lt;instancename&gt;/&lt;plugindir&gt; directory. But if you need
 * you may use a different directory
 *
 * You can add plugins from different KConfig[group], by just calling all times
 * you want addPlugins method with the correct parameters
 *
 * Additionally, calls to constructor with same @p categoryName, will add new
 * items to the same category, even if plugins are from different categories
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @author Rafael Fernández López <ereslibre@kde.org>
 * @deprecated Since 5.90, use KPluginWidget instead
 */
class KCMUTILS_EXPORT KPluginSelector : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList configurationArguments READ configurationArguments WRITE setConfigurationArguments)

public:
    enum PluginLoadMethod {
        ReadConfigFile = 0,
        IgnoreConfigFile,
    };

    /**
     * Create a new KPluginSelector
     */
    KCMUTILS_DEPRECATED_VERSION(5, 90, "Use KPluginWidget instead")
    KPluginSelector(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~KPluginSelector() override;

    /**
     * Add a list of KParts plugins
     *
     * The information about the plugins will be loaded from the
     * share/apps/&lt;instancename&gt;/kpartplugins directory
     *
     * @param componentName The name of the component of the plugin's parent.
     * @param categoryName  The translated name of the category. This is the
     *                      name that is shown in the title. If the category
     *                      did exist before because of another call to
     *                      addPlugins, then they will be shown in that
     *                      category. If @p categoryName is a new one, then
     *                      a new category will be shown on the plugin window,
     *                      and the list of plugins added to it
     * @param categoryKey   When you have different categories of KParts
     *                      plugins you distinguish between the plugins using
     *                      the Category key in the .desktop file. Use this
     *                      parameter to select only those KParts plugins
     *                      with the Category key == @p categoryKey. If
     *                      @p categoryKey is not set the Category key is
     *                      ignored and all plugins are shown. Not match case
     * @param config        The KConfig object that holds the state of the
     *                      plugins being enabled or not. By default it will be
     *                      set to KSharedConfig::openConfig(componentName + "rc").
     */
    void addPlugins(const QString &componentName,
                    const QString &categoryName = QString(),
                    const QString &categoryKey = QString(),
                    KSharedConfig::Ptr config = KSharedConfig::Ptr());

    /**
     * Add a list of non-KParts plugins
     *
     * @param pluginInfoList   A list of KPluginInfo objects containing the
     *                         necessary information for the plugins you want to
     *                         add to the list
     * @param pluginLoadMethod If KPluginSelector will try to load the
     *                         state of the plugin when loading the
     *                         dialog from the configuration file or not.
     *                         This is useful if for some reason you
     *                         called the setPluginEnabled() for each plugin
     *                         individually before loading the dialog, and
     *                         don't want KPluginSelector to override them
     *                         when loading
     * @param categoryName     The translated name of the category. This is the
     *                         name that is shown in the title. If the category
     *                         did exist before because of another call to
     *                         addPlugins, then they will be shown in that
     *                         category. If @p categoryName is a new one, then
     *                         a new category will be shown on the plugin window,
     *                         and the list of plugins added to it
     * @param categoryKey      When you have different categories of KParts
     *                         plugins you distinguish between the plugins using
     *                         the Category key in the .desktop file. Use this
     *                         parameter to select only those KParts plugins
     *                         with the Category key == @p categoryKey. If
     *                         @p categoryKey is not set the Category key is
     *                         ignored and all plugins are shown. Not match case
     * @param config           The KConfig object that holds the state of the
     *                         plugins being enabled or not. By default it will
     *                         use KSharedConfig::openConfig(). It is recommended to
     *                         always pass a KConfig object if you use
     *                         KSettings::PluginPage since you never know from
     *                         where the page will be called (think global
     *                         config app). For example KViewCanvas passes
     *                         KConfig("kviewcanvas")
     *
     * @note   All plugins that were set a config group using setConfig() method
     *         will load and save their information from there. For those that
     *         weren't any config object, @p config will be used
     */
    void addPlugins(const QList<KPluginInfo> &pluginInfoList,
                    PluginLoadMethod pluginLoadMethod = ReadConfigFile,
                    const QString &categoryName = QString(),
                    const QString &categoryKey = QString(),
                    const KSharedConfig::Ptr &config = KSharedConfig::Ptr());

    /**
     * Remove all plugins from the entry list.
     * @since 5.73
     */
    void clearPlugins();

    /**
     * Load the state of the plugins (selected or not) from the KPluginInfo
     * objects
     */
    void load();

    /**
     * Save the configuration
     */
    void save();

    /**
     * Returns true if the plugin selector has any changes that are not yet saved to configuration.
     * @see save()
     * @since 5.78
     */
    bool isSaveNeeded() const;

    /**
     * Change to applications defaults
     * @see isDefault()
     */
    void defaults();

    /**
     * Returns true if the plugin selector does not have any changes to application defaults
     * @see defaults()
     * @since 4.3
     */
    bool isDefault() const;

    /**
     * Updates plugins state (enabled or not)
     *
     * This method won't save anything on any configuration file. It will just
     * be useful if you added plugins with the method:
     *
     * \code
     * void addPlugins(const QList<KPluginInfo> &pluginInfoList,
     *                 const QString &categoryName = QString(),
     *                 const QString &categoryKey = QString(),
     *                 const KSharedConfig::Ptr &config = KSharedConfig::Ptr());
     * \endcode
     *
     * To sum up, this method will update your plugins state depending if plugins
     * are ticked or not on the KPluginSelector dialog, without saving anything
     * anywhere
     */
    void updatePluginsState();

    /**
     * Sets the @p arguments with which the configuration modules will be initialized
     *
     * @since 5.9
     */
    void setConfigurationArguments(const QStringList &arguments);

    /**
     * Returns the configuration arguments that will be used
     *
     * @since 5.9
     */
    QStringList configurationArguments() const;

    /**
     * Shows the configuration dialog for the plugin @p pluginId if it's available
     *
     * @since 5.45
     */
    void showConfiguration(const QString &pluginId);

    /**
     * Add additional widgets to each row of the plugin selector
     * @param handler returns the additional button that should be displayed in the row
     * the handler can return a null pointer if no button should be displayed
     * @since 5.74
     */
    void setAdditionalButtonHandler(std::function<QPushButton *(const KPluginInfo &)> handler);

    /**
     * Show an indicator when a plugin status is different from default
     *
     * @since 5.78
     */
    void setDefaultsIndicatorsVisible(bool isVisible);

Q_SIGNALS:
    /**
     * Tells you whether the configuration is changed or not.
     */
    void changed(bool hasChanged);

    /**
     * Emitted after the config of an embedded KCM has been saved. The
     * argument is the name of the parent component that needs to reload
     * its config
     */
    void configCommitted(const QByteArray &componentName);

    /**
     * Emitted after configuration is changed, tell if configuration represent default or not
     * @since 5.67
     */
    void defaulted(bool isDefault);

    /**
     * Emitted when show defaults indicators changed
     * @see setDefaultsIndicatorsVisible
     *
     * @since 5.78
     */
    void defaultsIndicatorsVisible();

private:
    class Private;
    Private *const d;
};

#endif
#endif
