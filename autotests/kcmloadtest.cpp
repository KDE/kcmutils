/*
    SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KCModuleLoader>

#include <QObject>
#include <QTest>

class KCMTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testLoadQmlPlugin()
    {
        auto mod = KCModuleLoader::loadModule(KPluginMetaData(QStringLiteral("plasma/kcms/systemsettings/kcm_testqml")));
        QVERIFY(mod);
        QCOMPARE(mod->metaObject()->className(), "KCModuleQml");
    }
};

QTEST_MAIN(KCMTest)
#include "kcmloadtest.moc"
