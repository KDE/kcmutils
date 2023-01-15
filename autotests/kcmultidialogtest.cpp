/*
    This file is part of the KDE Frameworks
    SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <KCMultiDialog>

#include <QObject>
#include <QTest>

class KCMultiDialogTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testClear();
};

void KCMultiDialogTest::testClear()
{
    KCMultiDialog dialog;
    dialog.addModule(KPluginMetaData(QStringLiteral("fakekcm")));
    // Just verify that it doesn't crash
    dialog.clear();
}

QTEST_MAIN(KCMultiDialogTest)
#include "kcmultidialogtest.moc"
