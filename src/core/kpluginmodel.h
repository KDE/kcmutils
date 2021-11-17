#pragma once

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
    void defaults();
    bool isSaveNeeded();

    Q_SIGNAL void defaulted(bool isDefaulted);

private:
    bool isPluginEnabled(const KPluginMetaData &plugin) const;
    KPluginMetaData findConfig(const KPluginMetaData &plugin) const;

    QVector<KPluginMetaData> m_plugins;
    KConfigGroup m_config;
    QHash<QString, QString> m_categoryLabels;
    QHash<QString, bool> m_pendingStates;
};
