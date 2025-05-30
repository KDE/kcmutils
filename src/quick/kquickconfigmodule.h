/*
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2001 Michael Goffioul <kdeprint@swing.be>
    SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>
    SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KQUICKCONFIGMODULE_H
#define KQUICKCONFIGMODULE_H

#include "kcmutilsquick_export.h"

#include <QObject>
#include <QQmlComponent>
#include <QStringList>
#include <QVariant>

#include <KPluginFactory>
#include <KPluginMetaData>

#include <memory>
#include <qqmlintegration.h>

#include "kabstractconfigmodule.h"
#include "kquickconfigmoduleloader.h"

class QQuickItem;
class QQmlEngine;
class KQuickConfigModulePrivate;

/*!
 * \class KQuickConfigModule
 * \inmodule KCMUtilsQuick
 * \brief The base class for QtQuick configuration modules.
 *
 * Configuration modules are realized as plugins that are dynamically loaded.
 *
 * All the necessary glue logic and the GUI bells and whistles
 * are provided by the control center and must not concern
 * the module author.
 *
 * To write a config module, you have to create a C++ plugin
 * and an accompanying QML user interface.
 *
 * To allow KCMUtils to load your ConfigModule subclass, you must create a KPluginFactory implementation.
 *
 * \code
 * #include <KPluginFactory>
 *
 * K_PLUGIN_CLASS_WITH_JSON(MyConfigModule, "yourmetadata.json")
 * \endcode
 *
 * The constructor of the ConfigModule then looks like this:
 * \code
 * YourConfigModule::YourConfigModule(QObject *parent, const KPluginMetaData &metaData)
 *   : KQuickConfigModule(parent, metaData)
 * {
 * }
 * \endcode
 *
 * The QML part must be in the KPackage format, installed under share/kpackage/kcms.
 * \sa KPackage::Package
 *
 * The package must have the same name as the plugin filename, to be installed
 * by CMake with the command:
 * \badcode
 * kpackage_install_package(packagedir kcm_yourconfigmodule kcms)
 * \endcode
 * The "packagedir" is the subdirectory in the source tree where the package sources are
 * located, and "kcm_yourconfigmodule" is id of the plugin.
 * Finally "kcms" is the literal string "kcms", so that the package is
 * installed as a configuration module (and not some other kind of package).
 *
 * The QML part can access all the properties of ConfigModule (together with the properties
 * defined in its subclass) by accessing to the global object "kcm", or with the
 * import of "org.kde.kcmutils" the ConfigModule attached property.
 *
 * \qml
 * import QtQuick
 * import QtQuick.Controls as QQC2
 * import org.kde.kcmutils as KCMUtils
 * import org.kde.kirigami as Kirigami
 *
 * Item {
 *     // implicit size will be used as initial size when loaded in kcmshell6
 *     implicitWidth: Kirigami.Units.gridUnit * 30
 *     implicitHeight: Kirigami.Units.gridUnit * 30
 *
 *     KCMUtils.ConfigModule.buttons: KCMUtils.ConfigModule.Help | KCMUtils.ConfigModule.Apply
 *
 *     QQC2.Label {
 *         // The following two bindings are equivalent:
 *         text: kcm.needsSave
 *         enabled: KCMUtils.ConfigModule.needsSave
 *     }
 * }
 * \endqml
 *
 * See https://develop.kde.org/docs/features/configuration/kcm/ for more detailed documentation.
 * \since 6.0
 */
class KCMUTILSQUICK_EXPORT KQuickConfigModule : public KAbstractConfigModule
{
    Q_OBJECT

    /*! \property KQuickConfigModule::mainUi */
    Q_PROPERTY(QQuickItem *mainUi READ mainUi CONSTANT)
    /*! \property KQuickConfigModule::columnWidth */
    Q_PROPERTY(int columnWidth READ columnWidth WRITE setColumnWidth NOTIFY columnWidthChanged)
    /*! \property KQuickConfigModule::depth */
    Q_PROPERTY(int depth READ depth NOTIFY depthChanged)
    /*! \property KQuickConfigModule::currentIndex */
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

    QML_NAMED_ELEMENT(ConfigModule)
    QML_ATTACHED(KQuickConfigModule)

public:
    ~KQuickConfigModule() override;

    /*!
     * \brief Returns the qml engine that built the main config UI.
     */
    std::shared_ptr<QQmlEngine> engine() const;

    /*!
     * \brief The error string in case the mainUi failed to load.
     */
    QString errorString() const;

    // QML property accessors

    /*!
     * \brief Returns the main UI for this configuration module.
     *
     * It's a QQuickItem coming from the QML package named
     * the same as the KAboutData's component name for this config module.
     *
     * Normally, ui/main.qml will be loaded from the qrc baked into the plugin. However,
     * if the PLASMA_PLATFORM environmental variable is set, the module will try to load
     * a platform-specific QML file as its mainUi starting point.
     *
     * For example:
     *
     * environment has set
     * \c {export PLASMA_PLATFORM=phone:handset}
     *
     * The module will try to find main_phone.qml, then main_handset.qml, then main.qml.
     * The first file that is found will be used as mainUi. If none is found and main.qml
     * doesn't exist, the module is broken and an error will be displayed.
     */
    QQuickItem *mainUi();

    /*!
     * \brief Returns a subpage at a given depth \a index.
     * \note This does not include the mainUi. That is, a depth of 2
     * is a mainUi and one subPage at index 0.
     */
    QQuickItem *subPage(int index) const;

    /*!
     * \brief Returns the width the kcm wants in column mode.
     *
     * If a columnWidth is valid ( > 0 ) and less than the systemsettings' view width,
     * more than one will be visible at once, and the first page will be a sidebar to the last page pushed.
     * As default, this is -1 which will make the shell always show only one page at a time.
     */
    int columnWidth() const;

    /*!
     * \brief Sets the column \a width we want.
     */
    void setColumnWidth(int width);

    /*!
     * \brief Returns how many pages this kcm has.
     *
     * It is guaranteed to be at least 1 (the main ui) plus how many times a new page has been pushed without pop.
     */
    int depth() const;

    /*!
     * \brief Sets the current page \a index this KCM should display.
     */
    void setCurrentIndex(int index);

    /*!
     * \brief Returns the index of the page this KCM should display.
     */
    int currentIndex() const;

    static KQuickConfigModule *qmlAttachedProperties(QObject *object);

public Q_SLOTS:
    /*!
     * \brief Push a new sub page with the given \a fileName
     * with optional \a initialProperties in the KCM hierarchy:
     * pages will be seen as a Kirigami PageRow.
     */
    void push(const QString &fileName, const QVariantMap &initialProperties = QVariantMap());

    /*!
     * \brief Push a given QQuick \a item as a new sub page in the KCM hierarchy.
     * \overload KQuickConfigModule::push()
     */
    void push(QQuickItem *item);

    /*!
     * \brief Pop the last page of the KCM hierarchy, the page is destroyed.
     */
    void pop();

    /*!
     * \brief Remove and return the last page of the KCM hierarchy.
     *
     * The popped page won't be deleted, it's the caller's responsibility
     * to manage the lifetime of the returned item.
     *
     * Returns the last page if any, \c nullptr otherwise.
     */
    QQuickItem *takeLast();

Q_SIGNALS:

    // QML NOTIFY signaling

    /*!
     * \brief Emitted when a new sub \a page is pushed.
     */
    void pagePushed(QQuickItem *page);

    // RFC: page argument?
    /*!
     * \brief Emitted when a sub page is popped.
     */
    void pageRemoved();

    /*!
     * \brief Emitted when the wanted column \a width of the KCM changes.
     */
    void columnWidthChanged(int width);

    /*!
     * \brief Emitted when the \a index of the current page changed.
     */
    void currentIndexChanged(int index);

    /*!
     * \brief Emitted when the number of pages changed, and so their \a index.
     */
    void depthChanged(int index);

    /*!
     * \brief Emitted when the main Ui has loaded successfully and mainUi() is available.
     */
    void mainUiReady();

protected:
    /*!
     * \brief Base class for all QtQuick config modules.
     *
     * Creates a new KQuickConfigModule with the given \a metaData as a child of \a parent.
     * Use KQuickConfigModuleLoader to instantiate this class.
     *
     * \note Do not emit changed signals here, since they are not yet connected to any slot.
     */
    explicit KQuickConfigModule(QObject *parent, const KPluginMetaData &metaData);

private:
    void setInternalEngine(const std::shared_ptr<QQmlEngine> &engine);
    friend KPluginFactory::Result<KQuickConfigModule>
    KQuickConfigModuleLoader::loadModule(const KPluginMetaData &metaData, QObject *parent, const QVariantList &args, const std::shared_ptr<QQmlEngine> &engine);
    const std::unique_ptr<KQuickConfigModulePrivate> d;
};

#endif // KQUICKCONFIGMODULE_H
