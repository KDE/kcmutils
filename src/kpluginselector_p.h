/**
  * This file is part of the KDE project
  * Copyright (C) 2007, 2006 Rafael Fernández López <ereslibre@kde.org>
  * Copyright (C) 2002-2003 Matthias Kretz <kretz@kde.org>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License version 2 as published by the Free Software Foundation.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#ifndef KPLUGINSELECTOR_P_H
#define KPLUGINSELECTOR_P_H

#include <QAbstractListModel>
#include <QAbstractItemDelegate>

#include <kconfiggroup.h>
#include <kplugininfo.h>
#include <kwidgetitemdelegate.h>
#include <kcategorizedsortfilterproxymodel.h>

class QLabel;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QAbstractItemView;

class KCategorizedView;
class KCModuleProxy;
class KCategoryDrawer;

class PluginEntry;

class Q_DECL_HIDDEN KPluginSelector::Private
    : public QObject
{
    Q_OBJECT

public:
    enum ExtraRoles {
        PluginEntryRole   = 0x09386561,
        ServicesCountRole = 0x1422E2AA,
        NameRole          = 0x0CBBBB00,
        CommentRole       = 0x19FC6DE2,
        AuthorRole        = 0x30861E10,
        EmailRole         = 0x02BE3775,
        WebsiteRole       = 0x13095A34,
        VersionRole       = 0x0A0CB450,
        LicenseRole       = 0x001F308A,
        DependenciesRole  = 0x04CAB650,
        IsCheckableRole   = 0x0AC2AFF8
    };

    Private(KPluginSelector *parent);
    ~Private();

    void updateDependencies(PluginEntry *pluginEntry, bool added);
    int dependantLayoutValue(int value, int width, int totalWidth) const;

public:
    class PluginModel;
    class ProxyModel;
    class PluginDelegate;
    class DependenciesWidget;
    KPluginSelector *parent;
    QLineEdit *lineEdit;
    KCategorizedView *listView;
    KCategoryDrawer *categoryDrawer;
    PluginModel *pluginModel;
    ProxyModel *proxyModel;
    PluginDelegate *pluginDelegate;
    DependenciesWidget *dependenciesWidget;
    bool showIcons;
    QStringList kcmArguments;
};

class PluginEntry
{
public:
    QString category;
    KPluginInfo pluginInfo;
    bool checked;
    bool manuallyAdded;
    KConfigGroup cfgGroup;
    KPluginSelector::PluginLoadMethod pluginLoadMethod;
    bool isCheckable;

    bool operator==(const PluginEntry &pe) const
    {
        // just comparing the entry path is not enough, since it is now also possible
        // to load the plugin information directly from a library (without .desktop files)
        return pluginInfo.entryPath() == pe.pluginInfo.entryPath()
            && pluginInfo.libraryPath() == pe.pluginInfo.libraryPath();
    }
};

Q_DECLARE_METATYPE(PluginEntry *)

/**
 * This widget will inform the user about changes that happened automatically
 * due to plugin dependencies.
 */
class KPluginSelector::Private::DependenciesWidget
    : public QWidget
{
    Q_OBJECT

public:
    DependenciesWidget(QWidget *parent = nullptr);
    ~DependenciesWidget();

    void addDependency(const QString &dependency, const QString &pluginCausant, bool added);
    void userOverrideDependency(const QString &dependency);

    void clearDependencies();

private Q_SLOTS:
    void showDependencyDetails();

private:
    struct FurtherInfo {
        bool added;
        QString pluginCausant;
    };

    void updateDetails();

    QLabel *details;
    QMap<QString, struct FurtherInfo> dependencyMap;
    int addedByDependencies;
    int removedByDependencies;
};

class KPluginSelector::Private::PluginModel
    : public QAbstractListModel
{
public:
    PluginModel(KPluginSelector::Private *pluginSelector_d, QObject *parent = nullptr);
    ~PluginModel();

    void addPlugins(const QList<KPluginInfo> &pluginList, const QString &categoryName, const QString &categoryKey, const KConfigGroup &cfgGroup, PluginLoadMethod pluginLoadMethod = ReadConfigFile, bool manuallyAdded = false);
    QList<KService::Ptr> pluginServices(const QModelIndex &index) const;

    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

public:
    QList<PluginEntry> pluginEntryList;

private:
    KPluginSelector::Private *pluginSelector_d;
};

class KPluginSelector::Private::ProxyModel
    : public KCategorizedSortFilterProxyModel
{
public:
    ProxyModel(KPluginSelector::Private *pluginSelector_d, QObject *parent = nullptr);
    ~ProxyModel();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool subSortLessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    KPluginSelector::Private *pluginSelector_d;
};

class KPluginSelector::Private::PluginDelegate
    : public KWidgetItemDelegate
{
    Q_OBJECT

public:
    PluginDelegate(KPluginSelector::Private *pluginSelector_d, QObject *parent = nullptr);
    ~PluginDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void configure(const QModelIndex &idx);

Q_SIGNALS:
    void changed(bool hasChanged);
    void configCommitted(const QByteArray &componentName);

protected:
    QList<QWidget *> createItemWidgets(const QModelIndex &index) const override;
    void updateItemWidgets(const QList<QWidget *> widgets,
                                   const QStyleOptionViewItem &option,
                                   const QPersistentModelIndex &index) const override;

private Q_SLOTS:
    void slotStateChanged(bool state);
    void emitChanged();
    void slotAboutClicked();
    void slotConfigureClicked();
    void slotDefaultClicked();

private:
    QFont titleFont(const QFont &baseFont) const;

    QCheckBox *checkBox;
    QPushButton *pushButton;
    QList<KCModuleProxy *> moduleProxyList;

    KPluginSelector::Private *pluginSelector_d;
};

#endif // KPLUGINSELECTOR_P_H
