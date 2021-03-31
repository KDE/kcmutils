/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007, 2006 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KPLUGINSELECTOR_P_H
#define KPLUGINSELECTOR_P_H

#include <QAbstractListModel>
#include <QSet>

#include <KCategorizedSortFilterProxyModel>
#include <KConfigGroup>
#include <KPluginInfo>
#include <kwidgetitemdelegate.h>

#include "kpluginmodel.h"

class QLabel;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QAbstractItemView;

class KCategorizedView;
class KCModuleProxy;
class KCategoryDrawer;

class PluginEntry;
class KPluginSelectorProxyModel;
class PluginDelegate;

class KPluginSelectorPrivate
{
public:
    int dependantLayoutValue(int value, int width, int totalWidth) const;

public:
    QLineEdit *lineEdit;
    KCategorizedView *listView;
    KCategoryDrawer *categoryDrawer;
    KPluginModel *pluginModel;
    KPluginSelectorProxyModel *proxyModel;
    QStringList kcmArguments;
};

class KPluginSelectorProxyModel : public KCategorizedSortFilterProxyModel
{
public:
    KPluginSelectorProxyModel(KPluginSelectorPrivate *pluginSelector_d, QObject *parent = nullptr);
    ~KPluginSelectorProxyModel();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool subSortLessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    KPluginSelectorPrivate *pluginSelector_d;
};

class PluginDelegate : public KWidgetItemDelegate
{
    Q_OBJECT

public:
    PluginDelegate(KPluginSelectorPrivate *pluginSelector_d, QObject *parent = nullptr);
    ~PluginDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void configure(const QModelIndex &idx);
    void setHandler(std::function<QPushButton *(const KPluginInfo &)> handler);

Q_SIGNALS:
    void changed(const QString &pluginId, bool enabled);
    void configCommitted(const QString &pluginId);

protected:
    QList<QWidget *> createItemWidgets(const QModelIndex &index) const override;
    void updateItemWidgets(const QList<QWidget *> widgets, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const override;

private Q_SLOTS:
    void slotStateChanged(bool state);
    void slotAboutClicked();
    void slotConfigureClicked();

private:
    QFont titleFont(const QFont &baseFont) const;

    QCheckBox *checkBox;
    QPushButton *pushButton;
    std::function<QPushButton *(const KPluginInfo &)> handler = nullptr;

    KPluginSelectorPrivate *pluginSelector_d;
};

#endif // KPLUGINSELECTOR_P_H
