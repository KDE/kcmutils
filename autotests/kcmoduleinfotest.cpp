/*
    This file is part of the KDE Frameworks
    SPDX-FileCopyrightText: 2020 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KCModuleInfo>
#include <KCMultiDialog>
#include <KPluginInfo>
#include <KPluginMetaData>
#include <QObject>
#include <QTest>

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")

class KCModuleInfoTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testExternalApp();
    void testFakeKCM();
    void testDesktopFileKCM();
    void testInvalidKCM();
};

#ifndef Q_OS_WIN
void initLocale()
{
    setenv("LC_ALL", "en_US.utf-8", 1);
}

Q_CONSTRUCTOR_FUNCTION(initLocale)
#endif

void KCModuleInfoTest::testExternalApp()
{
    const QString yast = QFINDTESTDATA("YaST-systemsettings.desktop");
    QVERIFY(!yast.isEmpty());
    KCModuleInfo info(yast);
    QVERIFY(info.service());
    QVERIFY(info.isValid());
}

void KCModuleInfoTest::testFakeKCM()
{
    // Similar to kontact's code
    // This is the case of loading a plugin, and then asking it for its kcmServices()
    // If there are none, Dialog::addPluginInfos still creates a fake KCM, so the plugin can be enabled/disabled.
    const QVector<KPluginMetaData> pluginMetaDatas = KPluginMetaData::findPlugins(QStringLiteral("testplugins"), [](const KPluginMetaData &) {
        return true;
    });
    const QList<KPluginInfo> pluginInfos = KPluginInfo::fromMetaData(pluginMetaDatas);
    QVERIFY(pluginInfos.count() > 0);
    KPluginInfo pluginInfo = pluginInfos.at(0);
    QVERIFY(pluginInfo.isValid());

    // WHEN
    KCModuleInfo info(pluginInfo); // like Dialog::addPluginInfos does

    // THEN
    QVERIFY(info.isValid());
    QCOMPARE(info.pluginInfo().name(), QStringLiteral("Test"));

#ifdef Q_OS_WIN
    QCOMPARE(QFileInfo(info.library()).fileName(), QStringLiteral("jsonplugin.dll"));
    QCOMPARE(QFileInfo(info.fileName()).fileName(), QStringLiteral("jsonplugin.dll"));
#else
    QCOMPARE(QFileInfo(info.library()).fileName(), QStringLiteral("jsonplugin.so"));
    QCOMPARE(QFileInfo(info.fileName()).fileName(), QStringLiteral("jsonplugin.so"));
#endif

    QCOMPARE(info.icon(), QStringLiteral("view-pim-mail"));
    QCOMPARE(info.comment(), QStringLiteral("Test plugin"));
    QCOMPARE(info.docPath(), QStringLiteral("doc/path"));
    QVERIFY(!info.service());
}

void KCModuleInfoTest::testDesktopFileKCM()
{
    const QString desktopFile = QFINDTESTDATA("desktopfilekcm/kcmtest.desktop");
    QVERIFY(!desktopFile.isEmpty());

    // WHEN
    KCModuleInfo info(desktopFile);

    // THEN
    QVERIFY(info.isValid());
    QVERIFY(info.service());
    QVERIFY(!info.pluginInfo().isValid());
    QCOMPARE(QFileInfo(info.library()).fileName(), QStringLiteral("kcm_kded"));
    QCOMPARE(QFileInfo(info.fileName()).fileName(), QStringLiteral("kcmtest.desktop"));
    QCOMPARE(info.icon(), QStringLiteral("preferences-system-session-services"));
    QCOMPARE(info.comment(), QStringLiteral("Configure background services"));
    QCOMPARE(info.docPath(), QStringLiteral("kcontrol/kded/index.html"));

#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 85)
    // WHEN actually loading the module
    KCMultiDialog dlg;
    QVERIFY(dlg.addModule(info));
#endif
}

void KCModuleInfoTest::testInvalidKCM()
{
    KCModuleInfo info(QStringLiteral("doest_not_exist.desktop"));
    QVERIFY(!info.isValid());
    QVERIFY(!info.service());
}

QTEST_MAIN(KCModuleInfoTest)
#include "kcmoduleinfotest.moc"
