#include "kpluginmodel.h"

#include "kcategorizedsortfilterproxymodel.h"

KPluginModel::KPluginModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant KPluginModel::data(const QModelIndex &index, int role) const
{
    const KPluginMetaData &plugin = m_plugins[index.row()];

    switch (role) {
    case Roles::NameRole:
        return plugin.name();
    case Roles::DescriptionRole:
        return plugin.description();
    case Roles::IconRole:
        return plugin.iconName();
    case Roles::EnabledRole:
        return isPluginEnabled(plugin);
    case Roles::IsChangeableRole:
        if (m_config.isValid()) {
            return !m_config.isEntryImmutable(plugin.pluginId() + QLatin1String("Enabled"));
        }
        return true;
    case MetaDataRole:
        return QVariant::fromValue(plugin);
    case KCategorizedSortFilterProxyModel::CategoryDisplayRole:
    case KCategorizedSortFilterProxyModel::CategorySortRole:
        return m_categoryLabels[plugin.pluginId()];
    case ConfigRole:
        return plugin.value(QStringLiteral("X-KDE-ConfigModule"));
    case IdRole:
        return plugin.pluginId();
    case EnabledByDefaultRole:
        return plugin.isEnabledByDefault();
    }

    qDebug() << "trying to get" << role;

    return {};
}

bool KPluginModel::isPluginEnabled(const KPluginMetaData &plugin) const
{
    if (m_pendingStates.contains(plugin.pluginId())) {
        return m_pendingStates[plugin.pluginId()];
    }

    if (m_config.isValid()) {
        return m_config.readEntry(plugin.pluginId() + QLatin1String("Enabled"), plugin.isEnabledByDefault());
    }
    return plugin.isEnabledByDefault();
}

bool KPluginModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Roles::EnabledRole) {
        m_pendingStates[m_plugins[index.row()].pluginId()] = value.toBool();
        Q_EMIT dataChanged(index, index, {Roles::EnabledRole});
        return true;
    }

    return false;
}

int KPluginModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_plugins.count();
}

void KPluginModel::addPlugins(const QVector<KPluginMetaData> &newPlugins, const QString &categoryLabel)
{
    beginInsertRows({}, m_plugins.size(), m_plugins.size() + newPlugins.size() - 1);
    m_plugins.append(newPlugins);

    for (const KPluginMetaData &plugin : newPlugins) {
        m_categoryLabels[plugin.pluginId()] = categoryLabel;
    }

    endInsertRows();
}

void KPluginModel::setConfig(const KConfigGroup &config)
{
    m_config = config;

    if (!m_plugins.isEmpty()) {
        Q_EMIT dataChanged(index(0, 0), index(m_plugins.size() - 1, 0), {Roles::EnabledRole, Roles::IsChangeableRole});
    }
}

void KPluginModel::clear()
{
    beginRemoveRows({}, 0, m_plugins.size() - 1);
    m_plugins.clear();
    endRemoveRows();
}

void KPluginModel::save()
{
    if (m_config.isValid()) {
        for (auto it = m_pendingStates.cbegin(); it != m_pendingStates.cend(); ++it) {
            return m_config.writeEntry(it.key() + QLatin1String("Enabled"), it.value());
        }
        m_config.sync();
    }
    m_pendingStates.clear();
}

void KPluginModel::defaults()
{
    m_pendingStates.clear();

    for (const KPluginMetaData &plugin : qAsConst(m_plugins)) {
        const bool changed = isPluginEnabled(plugin) != plugin.isEnabledByDefault();

        if (changed) {
            const int pluginIndex = m_plugins.indexOf(plugin);
            Q_EMIT dataChanged(index(pluginIndex, 0), index(pluginIndex, 0), {Roles::EnabledRole});
            if (m_config.isValid()) {
                m_config.deleteEntry(plugin.pluginId() + QLatin1String("Enabled"));
            }
        }
    }

    if (!m_plugins.isEmpty()) { }
}
