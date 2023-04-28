/*
   SPDX-FileCopyrightText: 2001 Waldo Bastian <bastian@kde.org>
   SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>

   SPDX-License-Identifier: LGPL-2.0-or-later

*/

#ifndef MAIN_H
#define MAIN_H

#include <KCMultiDialog>
#include <KPageDialog>
#include <QApplication>

class KCMShellMultiDialog : public KCMultiDialog
{
    Q_OBJECT

public:
    /**
     * Constructor. Parameter @p dialogFace is passed to KCMultiDialog
     * unchanged.
     */
    explicit KCMShellMultiDialog(KPageDialog::FaceType dialogFace, QWidget *parent = nullptr);
};

#endif // MAIN_H
