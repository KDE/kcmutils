/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007, 2006 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KPLUGINWIDGET_P_H
#define KPLUGINWIDGET_P_H

#include <QAbstractListModel>

#include <KCategorizedSortFilterProxyModel>
#include <KConfigGroup>
#include <KPluginInfo>
#include <kwidgetitemdelegate.h>

#include <kcmutils_export.h>

#include "kpluginmodel_p.h"

class QAbstractItemView;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;

class KCategorizedView;
class KCategoryDrawer;
class KCModuleProxy;

class KPluginWidgetProxyModel;
class PluginDelegate;
class PluginEntry;

class KPluginWidgetPrivate
{
public:
    int dependantLayoutValue(int value, int width, int totalWidth) const;

public:
    QLineEdit *lineEdit = nullptr;
    KCategorizedView *listView = nullptr;
    KCategoryDrawer *categoryDrawer = nullptr;
    KPluginModel *pluginModel = nullptr;
    KPluginWidgetProxyModel *proxyModel = nullptr;
    QStringList kcmArguments;
    bool showDefaultIndicator = false;
};

class KPluginWidgetProxyModel : public KCategorizedSortFilterProxyModel
{
public:
    explicit KPluginWidgetProxyModel(KPluginWidgetPrivate *pluginSelector_d, QObject *parent = nullptr);
    ~KPluginWidgetProxyModel() override;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool subSortLessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    KPluginWidgetPrivate *pluginSelector_d;
};

class PluginDelegate : public KWidgetItemDelegate
{
    Q_OBJECT

public:
    explicit PluginDelegate(KPluginWidgetPrivate *pluginSelector_d, QObject *parent = nullptr);
    ~PluginDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void configure(const QModelIndex &idx);

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

    KPluginWidgetPrivate *pluginSelector_d;

public:
    std::function<QPushButton *(const KPluginMetaData &)> handler;
};

#endif // KPLUGINSELECTOR_P_H
