/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "dispatcher.h"
#include "dispatcher_p.h"

#include <kcmutils_debug.h>

namespace KSettings
{

namespace Dispatcher
{

Q_GLOBAL_STATIC(DispatcherPrivate, d)

void registerComponent(const QString &componentName, QObject *recv, const char *slot)
{
    Q_ASSERT(!componentName.isEmpty());
    // qDebug() << componentName;
    d()->m_componentName[recv] = componentName;
    d()->m_componentInfo[componentName].slotList.append(ComponentInfo::Slot(recv, slot));

    ++(d()->m_componentInfo[componentName].count);
    QObject::connect(recv, &QObject::destroyed, d(), &DispatcherPrivate::unregisterComponent);
}

KSharedConfig::Ptr configForComponentName(const QString &componentName)
{
    // qDebug() ;
    return KSharedConfig::openConfig(componentName + QStringLiteral("rc"));
}

QList<QString> componentNames()
{
    // qDebug() ;
    QList<QString> names;
    for (QMap<QString, ComponentInfo>::ConstIterator it = d()->m_componentInfo.constBegin(), total = d()->m_componentInfo.constEnd(); it != total; ++it) {
        if ((*it).count > 0) {
            names.append(it.key());
        }
    }
    return names;
}

void reparseConfiguration(const QString &componentName)
{
    // qDebug() << componentName;
    // check if the componentName is valid:
    if (! d()->m_componentInfo.contains(componentName)) {
        return;
    }
    // first we reparse the config so that the KConfig object will be up to date
    KSharedConfig::Ptr config = configForComponentName(componentName);
    config->reparseConfiguration();

    const auto lstSlot = d()->m_componentInfo[componentName].slotList;
    for (const ComponentInfo::Slot &slot : lstSlot) {
        QMetaObject::invokeMethod(slot.first, slot.second);
    }
}

void syncConfiguration()
{
    for (QMap<QString, ComponentInfo>::ConstIterator it = d()->m_componentInfo.constBegin(), total = d()->m_componentInfo.constEnd(); it != total; ++it) {
        KSharedConfig::Ptr config = configForComponentName(it.key());
        config->sync();
    }
}

void DispatcherPrivate::unregisterComponent(QObject *obj)
{
    if (!m_componentName.contains(obj)) {
        qCWarning(KCMUTILS_LOG) << Q_FUNC_INFO << "Tried to unregister an object which is not already registered.";
        return;
    }

    QString name = m_componentName[obj];
    m_componentName.remove(obj); //obj will be destroyed when we return, so we better remove this entry
    --(m_componentInfo[name].count);
    // qDebug() << "componentName=" << name << "refcount=" << m_componentInfo[name].count;
    Q_ASSERT(m_componentInfo[name].count >= 0);
    if (m_componentInfo[name].count == 0) {
        m_componentInfo.remove(name);
    }
}

} // namespace Dispatcher
} // namespace KSettings

#include "moc_dispatcher_p.cpp"
