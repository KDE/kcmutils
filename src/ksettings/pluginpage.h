/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSETTINGS_PLUGINPAGE_H
#define KSETTINGS_PLUGINPAGE_H

#include <KCModule>
#include <kcmutils_export.h>

class KPluginSelector;

namespace KSettings
{
class PluginPagePrivate;

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 76)

/**
 * @short Convenience KCModule for creating a plugins config page.
 *
 * This class makes it very easy to create a plugins configuration page to your
 * program. All you need to do is create a class that is derived from
 * PluginPage and add the appropriate plugin information to the KPluginSelector.
 * This is done using the pluginSelector() method:
 * \code
 * K_PLUGIN_FACTORY(MyAppPluginConfigFactory,
 *                  registerPlugin<MyAppPluginConfig>();
 *                  )
 *
 * MyAppPluginConfig(QWidget * parent, const QVariantList & args)
 *     : PluginPage(MyAppPluginConfigFactory::componentData(), parent, args)
 * {
 *     pluginSelector()->addPlugins( QCoreApplication::instance()->applicationName(), i18n( "General Plugins" ), "General" );
 *     pluginSelector()->addPlugins( QCoreApplication::instance()->applicationName(), i18n( "Effects" ), "Effects" );
 * }
 * \endcode
 *
 * All that remains to be done is to create the appropriate .desktop file
 * \verbatim
   [Desktop Entry]
   Icon=plugin
   Type=Service
   X-KDE-ServiceTypes=KCModule

   X-KDE-Library=myapppluginconfig
   X-KDE-FactoryName=MyAppPluginConfigFactory
   X-KDE-ParentApp=myapp
   X-KDE-ParentComponents=myapp

   Name=Plugins
   Comment=Select and configure your plugins:
   \endverbatim
 *
 * @author Matthias Kretz <kretz@kde.org>
 * @deprecated since 5.76, use KPluginWidget instead.
 */
class KCMUTILS_EXPORT PluginPage : public KCModule
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(PluginPage)
public:
    /**
     * Standard KCModule constructor.
     * Automatically creates the KPluginSelector widget.
     */
    KCMUTILS_DEPRECATED_VERSION(5, 76, "Use KPluginWidget instead")
    explicit PluginPage(const KAboutData *aboutData, QWidget *parent = nullptr, const QVariantList &args = QVariantList());

    ~PluginPage() override;

    /**
     * @return a reference to the KPluginSelector.
     */
    KPluginSelector *pluginSelector();

    /**
     * Load the state of the plugins (selected or not) from the KPluginInfo
     * objects. For KParts plugins everything should work automatically. For
     * your own type of plugins you might need to reimplement the
     * KPluginInfo::pluginLoaded() method. If that doesn't fit your needs
     * you can also reimplement this method.
     */
    void load() override;

    /**
     * Save the state of the plugins to KConfig objects
     */
    void save() override;
    void defaults() override;

protected:
    PluginPagePrivate *const d_ptr;

private:
    Q_PRIVATE_SLOT(d_func(), void _k_reparseConfiguration(const QByteArray &a))
};

#endif

}

#endif // KSETTINGS_PLUGINPAGE_H
