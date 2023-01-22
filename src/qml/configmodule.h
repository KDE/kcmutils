/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2001 Michael Goffioul <kdeprint@swing.be>
    SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>
    SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CONFIGMODULE_H
#define CONFIGMODULE_H

#include "kcmutilsqml_export.h"

#include <QObject>
#include <QQmlComponent>
#include <QStringList>
#include <QVariant>

#include <KPluginMetaData>

#include <memory>

#include "kabstractconfigmodule.h"

class QQuickItem;
class QQmlEngine;

namespace KQuickAddons
{
class ConfigModulePrivate;

/**
 * @class KQuickAddons::ConfigModule configmodule.h KQuickAddons/ConfigModule
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
 * To write a config module, you have to create a C++ library
 * and an accompaning QML user interface.
 * The library must contain a factory function like the following:
 *
 * \code
 * #include <KPluginFactory>
 *
 * K_PLUGIN_FACTORY(MyConfigModuleFactory, registerPlugin<MyConfigModule>() )
 * \endcode
 *
 * The constructor of the ConfigModule then looks like this:
 * \code
 * YourConfigModule::YourConfigModule( QObject* parent )
 *   : ConfigModule( parent )
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
 * The QML part must be in the KPackage format, installed under share/kpackage/kcms.
 * @see KPackage::Package
 *
 * The package must have the same name as the KAboutData componentName, to be installed
 * by CMake with the command:
 * \code
 * kpackage_install_package(packagedir kcm_componentName kcms)
 * \endcode
 * The "packagedir" is the subdirectory in the source tree where the package sources are
 * located, and "kcm_componentName" is the componentname passed to the KAboutData in the
 * C++ part. Finally "kcms" is the literal string "kcms", so that the package is
 * installed as a configuration module (and not some other kind of package).
 * The main config dialog UI will be the file
 * ui/main.qml from the package (or what X-KPackage-MainScript value is in the
 * package metadata desktop file).
 *
 * The QML part can access all the properties of ConfigModule (together with the properties
 * defined in its subclass) by accessing to the global object "kcm", or with the
 * import of "org.kde.kcm 1.0" the ConfigModule attached property.
 *
 * \code
 * import QtQuick 2.1
 * import QtQuick.Controls 1.0 as QtControls
 * import org.kde.kcm 1.0
 * import org.kde.plasma.core 2.0 as PlasmaCore
 *
 * Item {
 *     //implicitWidth and implicitHeight will be used as initial size
 *     //when loaded in kcmshell5
 *     implicitWidth: units.gridUnit * 20
 *     implicitHeight: units.gridUnit * 20
 *
 *     ConfigModule.buttons: ConfigModule.Help|ConfigModule.Apply
 *     Label {
 *         text: kcm.needsSave
 *     }
 * }
 * \endcode
 *
 * See https://techbase.kde.org/Development/Tutorials/KCM_HowTo
 * for more detailed documentation.
 *
 */
class KCMUTILSQML_EXPORT ConfigModule : public KAbstractConfigModule
{
    Q_OBJECT

    Q_PROPERTY(QQuickItem *mainUi READ mainUi CONSTANT)
    Q_PROPERTY(int columnWidth READ columnWidth WRITE setColumnWidth NOTIFY columnWidthChanged)
    Q_PROPERTY(int depth READ depth NOTIFY depthChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

public:

    /**
     * Base class for all KControlModules.
     *
     * @note do not emit changed signals here, since they are not yet connected
     *       to any slot.
     */
    explicit ConfigModule(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args = QVariantList());

    /**
     * Base class for all KControlModules.
     *
     * @note do not emit changed signals here, since they are not yet connected
     *       to any slot.
     */
    explicit ConfigModule(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    /**
     * Destroys the module.
     */
    ~ConfigModule() override;

    /**
     * @return the qml engine that built the main config UI
     */
    std::shared_ptr<QQmlEngine> engine() const;

    /**
     * The status of the mainUi component.
     */
    QQmlComponent::Status status() const;

    /**
     * The error string in case the mainUi failed to load.
     * @return 5.64
     */
    QString errorString() const;

    // QML property accessors

    /**
     * @return The main UI for this configuration module. It's a QQuickItem coming from
     * the QML package named the same as the KAboutData's component name for
     * this config module
     */
    QQuickItem *mainUi();

    /*
     * @return a subpage at a given depth
     * @note This does not include the mainUi. i.e a depth of 2 is a mainUi and one subPage
     * at index 0
     */
    QQuickItem *subPage(int index) const;

    /**
     * returns the width the kcm wants in column mode.
     * If a columnWidth is valid ( > 0 ) and less than the systemsettings' view width,
     * more than one will be visible at once, and the first page will be a sidebar to the last page pushed.
     * As default, this is -1 which will make the shell always show only one page at a time.
     */
    int columnWidth() const;

    /**
     * Sets the column width we want.
     */
    void setColumnWidth(int width);

    /**
     * @returns how many pages this kcm has.
     * It is guaranteed to be at least 1 (the main ui) plus how many times a new page has been pushed without pop
     */
    int depth() const;

    /**
     * Sets the current page index this kcm should display
     */
    void setCurrentIndex(int index);

    /**
     * @returns the index of the page this kcm should display
     */
    int currentIndex() const;

    static ConfigModule *qmlAttachedProperties(QObject *object);


public Q_SLOTS:
    /**
     * Push a new sub page in the KCM hierarchy: pages will be seen as a Kirigami PageRow
     */
    void push(const QString &fileName, const QVariantMap &propertyMap = QVariantMap());

    /**
     *
     */
    void push(QQuickItem *item);

    /**
     * pop the last page of the KCM hierarchy, the page is destroyed
     */
    void pop();

    /**
     * remove and return the last page of the KCM hierarchy:
     * the popped page won't be deleted, it's the caller's responsibility to manage the lifetime of the returned item
     * @returns the last page if any, nullptr otherwise
     */
    QQuickItem *takeLast();

    /**
     * Ask the shell to show a passive notification
     * @param message The message text to display
     * @param timeout (optional) the timeout, either in milliseconds or the strings "short" and "long"
     * @param actionText (optional) The notification can have a button with this text
     * @param callBack (optional) If actionText is set and callBack is a JavaScript function, it will be called when the use clicks the button.
     */
    void showPassiveNotification(const QString &message,
                                 const QVariant &timeout = QVariant(),
                                 const QString &actionText = QString(),
                                 const QJSValue &callBack = QJSValue());


Q_SIGNALS:


    // QML NOTIFY signaling




    /**
     * Emitted when a new sub page is pushed
     */
    void pagePushed(QQuickItem *page);

    /**
     * Emitted when a sub page is popped
     */
    // RFC: page argument?
    void pageRemoved();

    /**
     * Emitted when the wanted column width of the kcm changes
     */
    void columnWidthChanged(int width);

    /**
     * Emitted when the current page changed
     */
    void currentIndexChanged(int index);

    /**
     * Emitted when the number of pages changed
     */
    void depthChanged(int index);

    /**
     * Emitted when the kcm wants the shell to display a passive notification
     */
    void passiveNotificationRequested(const QString &message, const QVariant &timeout, const QString &actionText, const QJSValue &callBack);


private:
    ConfigModulePrivate *const d;
};

}

QML_DECLARE_TYPEINFO(KQuickAddons::ConfigModule, QML_HAS_ATTACHED_PROPERTIES)

#endif // ConfigModule_H