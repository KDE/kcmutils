/*
    SPDX-FileCopyrightText: 2020 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QObject>
#include <QVariantList>

class KCMTest : public QObject
{
    Q_OBJECT
public:
     explicit KCMTest(QObject* parent, const QVariantList& foo = QVariantList());
};
