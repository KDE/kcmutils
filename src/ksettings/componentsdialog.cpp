/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "componentsdialog_p.h"
#include <kcmutils_debug.h>

#include <KConfig>
#include <KLocalizedString>
#include <KPluginInfo>
#include <KSeparator>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <QList>
#include <QTreeWidget>

namespace KSettings
{
class Q_DECL_HIDDEN ComponentsDialog::ComponentsDialogPrivate
{
public:
    QTreeWidget *listview;
    QFrame *infowidget;
    QLabel *iconwidget;
    QLabel *commentwidget;
    QLabel *descriptionwidget;
    QMap<QTreeWidgetItem *, KPluginInfo *> plugininfomap;
    QList<KPluginInfo *> plugininfolist;
};

ComponentsDialog::ComponentsDialog(QWidget *parent, const char *name)
    : QDialog(parent)
    , d(new ComponentsDialogPrivate)
{
    setObjectName(QString::fromLatin1(name));
    setModal(false);
    setWindowTitle(i18n("Select Components"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    QWidget *page = new QWidget(this);
    layout->addWidget(page);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
    connect(buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &ComponentsDialog::savePluginInfos);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ComponentsDialog::savePluginInfos);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);

    QHBoxLayout *hbox = new QHBoxLayout(page);
    hbox->setContentsMargins(0, 0, 0, 0);

    d->listview = new QTreeWidget(page);
    d->listview->setMinimumSize(200, 200);
    d->infowidget = new QFrame(page);
    d->infowidget->setMinimumSize(200, 200);

    QVBoxLayout *vbox = new QVBoxLayout(d->infowidget);
    vbox->setContentsMargins(0, 0, 0, 0);

    d->iconwidget = new QLabel(d->infowidget);
    vbox->addWidget(d->iconwidget);
    vbox->addWidget(new KSeparator(d->infowidget));
    d->commentwidget = new QLabel(d->infowidget);
    d->commentwidget->setWordWrap(true);
    vbox->addWidget(d->commentwidget);
    d->descriptionwidget = new QLabel(d->infowidget);
    d->descriptionwidget->setWordWrap(true);
    vbox->addWidget(d->descriptionwidget);

    d->listview->setAcceptDrops(false);

    connect(d->listview, &QTreeWidget::itemPressed, this, &ComponentsDialog::executed);
    connect(d->listview, &QTreeWidget::itemActivated, this, &ComponentsDialog::executed);
    // clang-format off
    connect(d->listview, SIGNAL(itemSelectionChanged(QTreeWidgetItem*,int)), this, SLOT(executed(QTreeWidgetItem*,int)));
    // clang-format on
}

ComponentsDialog::~ComponentsDialog()
{
    delete d;
}

void ComponentsDialog::addPluginInfo(KPluginInfo *info)
{
    d->plugininfolist.append(info);
}

void ComponentsDialog::setPluginInfos(const QMap<QString, KPluginInfo *> &plugininfos)
{
    for (QMap<QString, KPluginInfo *>::ConstIterator it = plugininfos.begin(), total = plugininfos.end(); it != total; ++it) {
        d->plugininfolist.append(it.value());
    }
}

void ComponentsDialog::setPluginInfos(const QList<KPluginInfo *> &plugins)
{
    d->plugininfolist = plugins;
}

void ComponentsDialog::show()
{
    // clear the treelist
    d->listview->clear();
    d->plugininfomap.clear();

    // construct the treelist
    for (QList<KPluginInfo *>::ConstIterator it = d->plugininfolist.constBegin(), total = d->plugininfolist.constEnd(); it != total; ++it) {
        (*it)->load();
        QTreeWidgetItem *item = new QTreeWidgetItem(d->listview, QStringList((*it)->name()));
        if (!(*it)->icon().isEmpty()) {
            item->setIcon(0, QIcon::fromTheme((*it)->icon()));
        }
        item->setCheckState(0, (*it)->isPluginEnabled() ? Qt::Checked : Qt::Unchecked);
        d->plugininfomap[item] = (*it);
    }
    QDialog::show();
}

void ComponentsDialog::executed(QTreeWidgetItem *item, int)
{
    // qDebug() ;
    if (item == nullptr) {
        return;
    }

    bool checked = (item->checkState(0) == Qt::Checked);

    // qDebug() << "it's a " << ( checked ? "checked" : "unchecked" )
    //    << " QCheckListItem" << endl;

    KPluginInfo *info = d->plugininfomap[item];
    info->setPluginEnabled(checked);
    // checkDependencies( info );
    // show info about the component on the right
    d->iconwidget->setPixmap(QIcon::fromTheme(info->icon()).pixmap(style()->pixelMetric(QStyle::PM_MessageBoxIconSize)));
    d->commentwidget->setText(info->comment());
    // d->descriptionwidget->setText( info->description() );
}

void ComponentsDialog::savePluginInfos()
{
    for (QList<KPluginInfo *>::ConstIterator it = d->plugininfolist.constBegin(), total = d->plugininfolist.constEnd(); it != total; ++it) {
        if ((*it)->config().isValid()) {
            (*it)->save();
            (*it)->config().sync();
        }
    }
}

} // namespace

#include "moc_componentsdialog_p.cpp"
