/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef COMPONENTSDIALOG_P_H
#define COMPONENTSDIALOG_P_H

#include <QDialog>
#include <kcmutils_export.h>

#include <QList>

class QString;
class KPluginInfo;
class QTreeWidgetItem;

namespace KSettings
{
/**
  Dialog for selecting which plugins should be active for an application. Set
  the list of available plugins with \ref setPluginInfos. The dialog will save the
  configuration on clicking ok or apply to the applications config file. Connect
  to the okClicked() and applyClicked() signals to be notified about
  configuration changes.
*/
class KCMUTILS_EXPORT ComponentsDialog : public QDialog
{
    Q_OBJECT
public:
    /**
      Create Dialog.

      @param parent parent widget
      @param name   name
    */
    explicit ComponentsDialog(QWidget *parent = nullptr, const char *name = nullptr);
    ~ComponentsDialog();

    /**
      Add a plugin that the dialog offers for selection.
    */
    void addPluginInfo(KPluginInfo *);
    /**
      Set list of plugins the dialog offers for selection. (Overwrites a previous list)
    */
    void setPluginInfos(const QMap<QString, KPluginInfo *> &plugininfos);
    /**
      Set list of plugins the dialog offers for selection. (Overwrites a previous list)
    */
    void setPluginInfos(const QList<KPluginInfo *> &plugins);

    /**
     * reimplemented
     */
    void show();

private Q_SLOTS:
    void executed(QTreeWidgetItem *, int);
    void savePluginInfos();

private:
    class ComponentsDialogPrivate;
    ComponentsDialogPrivate *const d;
};

}

#endif // COMPONENTSDIALOG_P_H
