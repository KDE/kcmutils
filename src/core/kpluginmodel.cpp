#include "kpluginmodel.h"

#include <KCategorizedSortFilterProxyModel>
#include <KPluginInfo>
#include <KServiceTypeTrader>
#include <utility>

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
        return QVariant::fromValue(findConfig(plugin));
    case IdRole:
        return plugin.pluginId();
    case EnabledByDefaultRole:
        return plugin.isEnabledByDefault();
    }

    return {};
}

bool KPluginModel::isPluginEnabled(const KPluginMetaData &plugin) const
{
    if (m_pendingStates.contains(plugin.pluginId())) {
        return m_pendingStates[plugin.pluginId()];
    }

    if (m_pendingDefaults.contains(plugin.pluginId())) {
        return plugin.isEnabledByDefault();
    }

    if (m_config.isValid()) {
        return m_config.readEntry(plugin.pluginId() + QLatin1String("Enabled"), plugin.isEnabledByDefault());
    }
    return plugin.isEnabledByDefault();
}

bool KPluginModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Roles::EnabledRole) {
        const QString pluginId = m_plugins[index.row()].pluginId();

        // If we already have a pending state and the user reverts it remove it from the map
        if (m_pendingStates.contains(pluginId)) {
            if (m_pendingStates.value(pluginId) != value.toBool()) {
                m_pendingStates.remove(pluginId);
            }
        } else {
            m_pendingStates[pluginId] = value.toBool();
        }

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
            m_config.writeEntry(it.key() + QLatin1String("Enabled"), it.value());
        }

        for (const QString &pluginId : std::as_const(m_pendingDefaults)) {
            m_config.deleteEntry(pluginId + QLatin1String("Enabled"));
        }

        m_config.sync();
    }
    m_pendingStates.clear();
    m_pendingDefaults.clear();
}

void KPluginModel::defaults()
{
    for (const KPluginMetaData &plugin : std::as_const(m_plugins)) {
        const bool changed = isPluginEnabled(plugin) != plugin.isEnabledByDefault();

        if (changed) {
            const int pluginIndex = m_plugins.indexOf(plugin);
            m_pendingDefaults.insert(plugin.pluginId());
            m_pendingStates.remove(plugin.pluginId());
            Q_EMIT dataChanged(index(pluginIndex, 0), index(pluginIndex, 0), {Roles::EnabledRole});
        }
    }

    m_pendingStates.clear();
}

KPluginMetaData KPluginModel::findConfig(const KPluginMetaData &plugin) const
{
    const QString metaDataKCM = plugin.value(QStringLiteral("X-KDE-ConfigModule"));

    if (!metaDataKCM.isEmpty()) {
        return KPluginMetaData(plugin.rawData(), metaDataKCM);
    }

    // TODO KF6 remove
    const auto services =
        KServiceTypeTrader::self()->query(QStringLiteral("KCModule"), QLatin1Char('\'') + plugin.pluginId() + QLatin1String("' in [X-KDE-ParentComponents]"));

    return services.isEmpty() ? KPluginMetaData() : KPluginInfo(services.constFirst()).toMetaData();
}

bool KPluginModel::isSaveNeeded()
{
    return !m_pendingStates.isEmpty() || !m_pendingDefaults.isEmpty();
}
