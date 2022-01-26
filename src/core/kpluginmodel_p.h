/*
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPLUGINMODEL_H
#define KPLUGINMODEL_H

#include <QAbstractListModel>
#include <QVector>

#include <KConfigGroup>
#include <KPluginMetaData>

class KPluginModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::DisplayRole,
        IconRole = Qt::DecorationRole,
        EnabledRole = Qt::CheckStateRole,
        DescriptionRole = Qt::UserRole + 1,
        UntranslatedKeywordsRole,
        KeywordsRole,
        IsChangeableRole,
        MetaDataRole,
        ConfigRole,
        IdRole,
        EnabledByDefaultRole,
    };

    explicit KPluginModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void addPlugins(const QVector<KPluginMetaData> &plugins, const QString &categoryLabel);
    void clear();
    void setConfig(const KConfigGroup &config);
    void save();
    void load();
    void defaults();
    bool isSaveNeeded();

    Q_SIGNAL void defaulted(bool isDefaulted);

private:
    bool isDefaulted();
    bool isPluginEnabled(const KPluginMetaData &plugin) const;
    KPluginMetaData findConfig(const KPluginMetaData &plugin) const;

    QVector<KPluginMetaData> m_plugins;
    QHash<QString, KPluginMetaData> m_pluginKcms;
    KConfigGroup m_config;
    QHash<QString, QString> m_categoryLabels;
    QHash<QString, bool> m_pendingStates;
};
#endif
