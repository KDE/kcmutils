/*
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpluginmodel_p.h"

#include <KCategorizedSortFilterProxyModel>
#include <KServiceTypeTrader>
#include <utility>

#include "kcmutils_debug.h"

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
        return QVariant::fromValue(m_pluginKcms.value(plugin.pluginId()));
    case IdRole:
        return plugin.pluginId();
    case EnabledByDefaultRole:
        return plugin.isEnabledByDefault();
    }

    return {};
}

bool KPluginModel::isPluginEnabled(const KPluginMetaData &plugin) const
{
    auto pendingState = m_pendingStates.constFind(plugin.pluginId());
    if (pendingState != m_pendingStates.constEnd()) {
        return pendingState.value();
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
        auto pendingStateIt = m_pendingStates.constFind(pluginId);
        if (pendingStateIt != m_pendingStates.constEnd()) {
            if (pendingStateIt.value() != value.toBool()) {
                m_pendingStates.erase(pendingStateIt);
            }
        } else {
            m_pendingStates[pluginId] = value.toBool();
        }

        Q_EMIT dataChanged(index, index, {Roles::EnabledRole});
        Q_EMIT defaulted(isDefaulted());

        return true;
    }

    return false;
}

int KPluginModel::rowCount(const QModelIndex & /*parent*/) const
{
    return m_plugins.count();
}

void KPluginModel::addPlugins(const QVector<KPluginMetaData> &newPlugins, const QString &categoryLabel)
{
    beginInsertRows({}, m_plugins.size(), m_plugins.size() + newPlugins.size() - 1);
    m_plugins.append(newPlugins);

    for (const KPluginMetaData &plugin : newPlugins) {
        m_categoryLabels[plugin.pluginId()] = categoryLabel;
        m_pluginKcms.insert(plugin.pluginId(), findConfig(plugin));
    }

    endInsertRows();

    Q_EMIT defaulted(isDefaulted());
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
    if (m_plugins.isEmpty()) {
        return;
    }
    beginRemoveRows({}, 0, m_plugins.size() - 1);
    m_plugins.clear();
    m_pluginKcms.clear();
    // In case of the "Reset"-button of the KCMs load is called again with the goal
    // of discarding all local changes. Consequently, the pending states have to be cleared here.
    m_pendingStates.clear();
    endRemoveRows();
}

void KPluginModel::save()
{
    if (m_config.isValid()) {
        for (auto it = m_pendingStates.cbegin(); it != m_pendingStates.cend(); ++it) {
            m_config.writeEntry(it.key() + QLatin1String("Enabled"), it.value());
        }

        m_config.sync();
    }
    m_pendingStates.clear();
}

void KPluginModel::defaults()
{
    for (int pluginIndex = 0, count = m_plugins.count(); pluginIndex < count; ++pluginIndex) {
        const KPluginMetaData plugin = m_plugins.at(pluginIndex);
        const bool changed = isPluginEnabled(plugin) != plugin.isEnabledByDefault();

        if (changed) {
            // If the entry was marked as changed, but we flip the value it is unchanged again
            if (m_pendingStates.remove(plugin.pluginId()) == 0) {
                // If the entry was not changed before, we have to mark it as changed
                m_pendingStates.insert(plugin.pluginId(), plugin.isEnabledByDefault());
            }
            Q_EMIT dataChanged(index(pluginIndex, 0), index(pluginIndex, 0), {Roles::EnabledRole});
        }
    }

    Q_EMIT defaulted(true);
}

KPluginMetaData KPluginModel::findConfig(const KPluginMetaData &plugin) const
{
    const QString metaDataKCM = plugin.value(QStringLiteral("X-KDE-ConfigModule"));

    if (!metaDataKCM.isEmpty()) {
        const QString absoluteKCMPath = QPluginLoader(metaDataKCM).fileName();
        // If we have a static plugin the file does not exist on disk
        // instead we query in the plugin namespace
        if (absoluteKCMPath.isEmpty()) {
            const int idx = metaDataKCM.lastIndexOf(QLatin1Char('/'));
            const QString pluginNamespace = metaDataKCM.left(idx);
            const QString pluginId = metaDataKCM.mid(idx + 1);
            return KPluginMetaData::findPluginById(pluginNamespace, pluginId);
        } else {
            return KPluginMetaData(plugin.rawData(), absoluteKCMPath);
        }
    }

#if KSERVICE_BUILD_DEPRECATED_SINCE(5, 89)
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
    QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")

    const QString constraint = QLatin1Char('\'') + plugin.pluginId() + QLatin1String("' in [X-KDE-ParentComponents]");
    const auto services = KServiceTypeTrader::self()->query(QStringLiteral("KCModule"), constraint);
    if (!services.isEmpty()) {
        const KService::Ptr service = services.constFirst();
        qCWarning(KCMUTILS_LOG) << service->entryPath() << "Querying the KCMs associated to a plugin using the X-KDE-ParentComponents is deprecated."
                                << "Instead define the X-KDE-ConfigModule with the namespace and plugin filen name.";
        QJsonObject obj;
        obj.insert(QLatin1String("KPlugin"), QJsonObject({{QLatin1String("Name"), service->name()}}));
        obj.insert(QLatin1String("X-KDE-PluginKeyword"), service->pluginKeyword());
        return KPluginMetaData(obj, service->library());
    }
    QT_WARNING_POP
#endif
    return KPluginMetaData();
}

bool KPluginModel::isSaveNeeded()
{
    return !m_pendingStates.isEmpty();
}

bool KPluginModel::isDefaulted()
{
    return std::all_of(m_plugins.cbegin(), m_plugins.cend(), [this](const KPluginMetaData &data) {
        return isPluginEnabled(data) == data.isEnabledByDefault();
    });
}
