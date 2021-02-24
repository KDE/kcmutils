/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef DISPATCHER_P_H
#define DISPATCHER_P_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QPair>

namespace KSettings
{
namespace Dispatcher
{
class ComponentInfo
{
public:
    typedef QPair<QObject *, const char *> Slot;
    QList<Slot> slotList;
    int count;

    ComponentInfo()
        : count(0)
    {
    }
};

class DispatcherPrivate : public QObject
{
    Q_OBJECT
public:
    QMap<QString, ComponentInfo> m_componentInfo;
    QMap<QObject *, QString> m_componentName;

public Q_SLOTS:
    void unregisterComponent(QObject *);
};

} // namespace Dispatcher
} // namespace KSettings
#endif // DISPATCHER_P_H
