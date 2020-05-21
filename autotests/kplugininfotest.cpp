/* This file is part of the KDE Frameworks
    Copyright (c) 2020 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QTest>
#include <QObject>
#include <KCModuleInfo>

class KPluginInfoTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testExternalApp();
};

void KPluginInfoTest::testExternalApp()
{
    const QString yast = QFINDTESTDATA("YaST-systemsettings.desktop");
    QVERIFY(!yast.isEmpty());
    KCModuleInfo info(yast);
    QVERIFY(info.service());
}

QTEST_MAIN(KPluginInfoTest)
#include "kplugininfotest.moc"
