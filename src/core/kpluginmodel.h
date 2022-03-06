/*
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPLUGINMODEL_H
#define KPLUGINMODEL_H

#include "kcmutilscore_export.h"

#include <QAbstractListModel>
#include <QVector>

#include <KConfigGroup>
#include <KPluginMetaData>
#include <memory>

class KPluginModelPrivate;

class KCMUTILSCORE_EXPORT KPluginModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::DisplayRole,
        IconRole = Qt::DecorationRole,
        EnabledRole = Qt::CheckStateRole,
        DescriptionRole = Qt::UserRole + 1,
        IsChangeableRole,
        MetaDataRole,
        ConfigRole,
        IdRole,
        EnabledByDefaultRole,
    };

    explicit KPluginModel(QObject *parent = nullptr);
    ~KPluginModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addPlugins(const QVector<KPluginMetaData> &plugins, const QString &categoryLabel);
    void clear();
    void setConfig(const KConfigGroup &config);
    void save();
    void load();
    void defaults();
    bool isSaveNeeded();

    Q_SIGNAL void defaulted(bool isDefaulted);

private:
    const std::unique_ptr<KPluginModelPrivate> d;
};
#endif
