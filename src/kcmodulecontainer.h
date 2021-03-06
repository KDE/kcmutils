/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCMODULECONTAINER_H
#define KCMODULECONTAINER_H

#include <QString>
#include <QStringList>

#include <KCModule>
#include <kcmutils_export.h>

class QWidget;

class KCModuleProxy;

#if KCMUTILS_ENABLE_DEPRECATED_SINCE(5, 66)
/**
 * @brief KCModuleContainer is a convenience class encapsulating several KCModules.
 *
 * The KCModuleContainer class is a convenience class for organizing a multiple set
 * of KCModule. KCModuleContainer is a sub class of KCModule and builds an interface mainly
 * consisting of a tab widget where each tab contains one of the modules specified via one of the
 * constructors. KCModuleContainer can handle modules which requires root permissions. What you
 * most likely want is the KCMODULECONTAINER macro. \n
 * Sometimes it is of interest to detect in runtime whether a module should be loaded or not. This
 * can be achieved by sub classing KCModuleContainer, doing the probing/testing checks and then manually
 * call addModule for each module which should be displayed. When all calls to addModule is done, call
 * finalize() which performs some necessary final steps.
 *
 * @deprecated since 5.66, no known users.
 *
 * @author Frans Englich <frans.englich@telia.com>
 */
class KCMUTILS_EXPORT KCModuleContainer : public KCModule
{
    Q_OBJECT
public:
    /**
     * Creates a KCModuleContainer with tabs, each one containing one of the
     * specified modules in @p mods.
     *
     * @param parent the parent QWidget.
     * @param mods The list of KCModules to be loaded. The name of each
     * KCModule is its service name, that is the name of the desktop file without
     * the ".desktop" part
     *
     * @deprecated since 5.66, no known users.
     */
    KCMUTILS_DEPRECATED_VERSION(5, 66, "No known users")
    KCModuleContainer(QWidget *parent, const QStringList &mods);

    /**
     * This is a convenience function, instead of building a QStringList you
     * can specify the modules in a comma separated QString. For example;
     * \code
     * KCModuleContainer* cont = KCModuleContainer( this, "kcm_misc", QString("kcm_energy, kcm_keyboard ,kcm_useraccount, kcm_mouse") );
     * \endcode
     * The other constructor takes its modules in a QStringlist which also can be constructed from a
     * string and thus you will have to be explicit on the data type.
     *
     * What you probably want is the KCMODULECONTAINER macro which builds an KCModule
     * for you, taking the modules you want as argument.
     *
     * @param parent The parent widget
     * @param mods The modules to load
     * @return The KCModule containing the requested modules.
     *
     * @deprecated since 5.66, no known users.
     */
    KCMUTILS_DEPRECATED_VERSION(5, 66, "No known users")
    explicit KCModuleContainer(QWidget *parent, const QString &mods = QString());

    /**
     * Adds the specified module to the tab widget. Setting the tab icon, text,
     * tool tip, connecting the signals is what it does.
     *
     * @param module the name of the module to add. The name is the desktop file's name
     * without the ".desktop" part.
     *
     * @deprecated since 5.66, no known users.
     */
    KCMUTILS_DEPRECATED_VERSION(5, 66, "No known users")
    void addModule(const QString &module);

    /**
     * Default destructor.
     */
    ~KCModuleContainer() override;

    /**
     * @reimp
     */
    void save() override;

    /**
     * @reimp
     */
    void load() override;

    /**
     * @reimp
     */
    void defaults() override;

private Q_SLOTS:

    /**
     * Enables/disables the Admin Mode button, as appropriate.
     */
    void tabSwitched(int);

    void moduleChanged(KCModuleProxy *proxy);

private:
    void init();

    class KCModuleContainerPrivate;
    KCModuleContainerPrivate *const d;
};

/**
 * This macro creates an factory declaration which when run creates an KCModule with specified
 * modules. For example:
 * \code
 * KCMODULECONTAINER("kcm_fonts,kcm_keyboard,kcm_foo", misc_modules)
 * \endcode
 * would create a KCModule with three tabs, each containing one of the specified KCMs. Each
 * use of the macro must be accompanied by a desktop file where the factory name equals
 * the second argument in the macro(in this example, misc_modules). \n
 * The module container takes care of testing the contained modules when being shown, as well
 * as when the module itself is asked whether it should be shown.
 *
 * @param modules the modules to put in the container
 * @param factoryName what factory name the module should have
 *
 * @deprecated since 5.66, no known users.
 */
// clang-format off
#define KCMODULECONTAINER(modules, factoryName) \
    class KCModuleContainer##factoryName : public KCModuleContainer \
    { \
    public: \
        KCModuleContainer##factoryName(QWidget *parent, const QVariantList &) \
            : KCModuleContainer(parent, QLatin1String(modules)) \
        { \
        } \
    }; \
    K_PLUGIN_FACTORY(KCModuleContainer##factoryName##Factory, \
                     registerPlugin<KCModuleContainer#factoryName>(); \
                    )
// clang-format on
#endif
#endif // KCMODULECONTAINER_H
