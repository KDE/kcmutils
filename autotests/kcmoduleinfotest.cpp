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
#include <KPluginLoader>
#include <KPluginMetaData>
#include <KPluginInfo>

class KCModuleInfoTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testExternalApp();
    void testFakeKCM();
};

void KCModuleInfoTest::testExternalApp()
{
    const QString yast = QFINDTESTDATA("YaST-systemsettings.desktop");
    QVERIFY(!yast.isEmpty());
    KCModuleInfo info(yast);
    QVERIFY(info.service());
}

void KCModuleInfoTest::testFakeKCM()
{
    // Similar to kontact's code
    const QVector<KPluginMetaData> pluginMetaDatas = KPluginLoader::findPlugins(
            QStringLiteral("testplugins"), [](const KPluginMetaData &) { return true; });
    const QList<KPluginInfo> pluginInfos = KPluginInfo::fromMetaData(pluginMetaDatas);
    QVERIFY(pluginInfos.count() > 0);
    KPluginInfo pluginInfo = pluginInfos.at(0);
    QVERIFY(pluginInfo.isValid());

    // WHEN
    KCModuleInfo info(pluginInfo); // like Dialog::addPluginInfos does

    // THEN
    QCOMPARE(info.pluginInfo().name(), QStringLiteral("Test"));
    QCOMPARE(QFileInfo(info.library()).fileName(), QStringLiteral("jsonplugin.so"));
    QCOMPARE(info.icon(), QStringLiteral("view-pim-mail"));
    QCOMPARE(info.comment(), QStringLiteral("Test plugin"));
    QCOMPARE(info.docPath(), QStringLiteral("doc/path"));
    QVERIFY(!info.service());
}

QTEST_MAIN(KCModuleInfoTest)
#include "kcmoduleinfotest.moc"
