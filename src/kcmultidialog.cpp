/*
   Copyright (c) 2000 Matthias Elter <elter@kde.org>
   Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (c) 2003,2006 Matthias Kretz <kretz@kde.org>
   Copyright (c) 2004 Frans Englich <frans.englich@telia.com>
   Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#include "kcmultidialog.h"
#include "kcmultidialog_p.h"

#include "kcmoduleproxy.h"

#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QStringList>
#include <QUrl>

#include <KAuthAction>
#include <KAuthObjectDecorator>
#include <KGuiItem>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPageWidgetModel>

bool KCMultiDialogPrivate::resolveChanges(KCModuleProxy *currentProxy)
{
    Q_Q(KCMultiDialog);
    if (!currentProxy || !currentProxy->changed()) {
        return true;
    }

    // Let the user decide
    const int queryUser = KMessageBox::warningYesNoCancel(
                              q,
                              i18n("The settings of the current module have changed.\n"
                                   "Do you want to apply the changes or discard them?"),
                              i18n("Apply Settings"),
                              KStandardGuiItem::apply(),
                              KStandardGuiItem::discard(),
                              KStandardGuiItem::cancel());

    switch (queryUser) {
    case KMessageBox::Yes:
        return moduleSave(currentProxy);

    case KMessageBox::No:
        currentProxy->load();
        return true;

    case KMessageBox::Cancel:
        return false;

    default:
        Q_ASSERT(false);
        return false;
    }
}

void KCMultiDialogPrivate::_k_slotCurrentPageChanged(KPageWidgetItem *current, KPageWidgetItem *previous)
{
    Q_Q(KCMultiDialog);
    // qDebug();

    q->blockSignals(true);
    q->setCurrentPage(previous);

    KCModuleProxy *previousModule = nullptr;
    for (int i = 0; i < modules.count(); ++i) {
        if (modules[ i ].item == previous) {
            previousModule = modules[ i ].kcm;
            break;
        }
    }

    if (resolveChanges(previousModule)) {
        q->setCurrentPage(current);
    }
    q->blockSignals(false);

    // We need to get the state of the now active module
    _k_clientChanged();
}

void KCMultiDialogPrivate::_k_clientChanged()
{
    Q_Q(KCMultiDialog);
    // qDebug();
    // Get the current module
    KCModuleProxy *activeModule = nullptr;
    for (int i = 0; i < modules.count(); ++i) {
        if (modules[ i ].item == q->currentPage()) {
            activeModule = modules[ i ].kcm;
            break;
        }
    }

    bool change = false;
    if (activeModule) {
        change = activeModule->changed();

        QPushButton *applyButton = q->buttonBox()->button(QDialogButtonBox::Apply);
        if (applyButton) {
            q->disconnect(applyButton, &QAbstractButton::clicked, q, &KCMultiDialog::slotApplyClicked);
            delete applyButton->findChild<KAuth::ObjectDecorator *>();
            applyButton->setEnabled(change);
        }

        QPushButton *okButton = q->buttonBox()->button(QDialogButtonBox::Ok);
        if (okButton) {
            q->disconnect(okButton, &QAbstractButton::clicked, q, &KCMultiDialog::slotOkClicked);
            delete okButton->findChild<KAuth::ObjectDecorator *>();
        }

        if (activeModule->realModule()->needsAuthorization()) {
            if (applyButton) {
                KAuth::ObjectDecorator *decorator = new KAuth::ObjectDecorator(applyButton);
                decorator->setAuthAction(activeModule->realModule()->authAction());
                activeModule->realModule()->authAction().setParentWidget(activeModule->realModule());
                q->connect(decorator, &KAuth::ObjectDecorator::authorized, q, &KCMultiDialog::slotApplyClicked);
            }

            if (okButton) {
                KAuth::ObjectDecorator *decorator = new KAuth::ObjectDecorator(okButton);
                decorator->setAuthAction(activeModule->realModule()->authAction());
                activeModule->realModule()->authAction().setParentWidget(activeModule->realModule());
                q->connect(decorator, &KAuth::ObjectDecorator::authorized, q, &KCMultiDialog::slotOkClicked);
            }
        } else {
            if (applyButton) {
                q->connect(applyButton, &QAbstractButton::clicked, q, &KCMultiDialog::slotApplyClicked);
                delete applyButton->findChild<KAuth::ObjectDecorator *>();
            }

            if (okButton) {
                q->connect(okButton, &QAbstractButton::clicked, q, &KCMultiDialog::slotOkClicked);
                delete okButton->findChild<KAuth::ObjectDecorator *>();
            }
        }
    }

    QPushButton *resetButton = q->buttonBox()->button(QDialogButtonBox::Reset);
    if (resetButton) {
        resetButton->setEnabled(change);
    }

    QPushButton *applyButton = q->buttonBox()->button(QDialogButtonBox::Apply);
    if (applyButton) {
        applyButton->setEnabled(change);
    }

    if (activeModule) {
        QPushButton *helpButton = q->buttonBox()->button(QDialogButtonBox::Help);
        if (helpButton) {
            helpButton->setEnabled(activeModule->buttons() & KCModule::Help);
        }

        QPushButton *defaultButton = q->buttonBox()->button(QDialogButtonBox::RestoreDefaults);
        if (defaultButton) {
            defaultButton->setEnabled(activeModule->buttons() & KCModule::Default);
        }
    }
}

void KCMultiDialogPrivate::_k_updateHeader(bool use, const QString &message)
{
    Q_Q(KCMultiDialog);
    KPageWidgetItem *item = q->currentPage();
    KCModuleProxy *kcm = qobject_cast<KCModuleProxy *>(item->widget());

    if (use) {
        item->setHeader(QStringLiteral("<b>") + kcm->moduleInfo().comment() + QStringLiteral("</b><br><i>") +
                        message + QStringLiteral("</i>"));
        item->setIcon(KDE::icon(kcm->moduleInfo().icon(), QStringList() << QStringLiteral("dialog-warning")));
    } else {
        item->setHeader(kcm->moduleInfo().comment());
        item->setIcon(QIcon::fromTheme(kcm->moduleInfo().icon()));
    }
}

void KCMultiDialogPrivate::init()
{
    Q_Q(KCMultiDialog);
    q->setFaceType(KPageDialog::Auto);
    q->setWindowTitle(i18n("Configure"));
    q->setModal(false);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(q);
    buttonBox->setStandardButtons(QDialogButtonBox::Help
                                  | QDialogButtonBox::RestoreDefaults
                                  | QDialogButtonBox::Cancel
                                  | QDialogButtonBox::Apply
                                  | QDialogButtonBox::Ok
                                  | QDialogButtonBox::Reset);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::RestoreDefaults), KStandardGuiItem::defaults());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Apply), KStandardGuiItem::apply());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Reset), KStandardGuiItem::reset());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Help), KStandardGuiItem::help());
    buttonBox->button(QDialogButtonBox::Reset)->setEnabled(false);
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

    q->connect(buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, q, &KCMultiDialog::slotApplyClicked);
    q->connect(buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::clicked, q, &KCMultiDialog::slotOkClicked);
    q->connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, q, &KCMultiDialog::slotDefaultClicked);
    q->connect(buttonBox->button(QDialogButtonBox::Help), &QAbstractButton::clicked, q, &KCMultiDialog::slotHelpClicked);
    q->connect(buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, q, &KCMultiDialog::slotUser1Clicked);

    q->setButtonBox(buttonBox);

    q->connect(q, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
               SLOT(_k_slotCurrentPageChanged(KPageWidgetItem*,KPageWidgetItem*)));
}

KCMultiDialog::KCMultiDialog(QWidget *parent)
    : KPageDialog(parent)
    , d_ptr(new KCMultiDialogPrivate(this))
{
    d_func()->init();
}

KCMultiDialog::KCMultiDialog(KPageWidget *pageWidget, QWidget *parent, Qt::WindowFlags flags)
    : KPageDialog(pageWidget, parent, flags)
    , d_ptr(new KCMultiDialogPrivate(this))
{
    d_func()->init();
}

KCMultiDialog::KCMultiDialog(KCMultiDialogPrivate &dd, KPageWidget *pageWidget, QWidget *parent, Qt::WindowFlags flags)
    : KPageDialog(pageWidget, parent, flags)
    , d_ptr(&dd)
{
    d_func()->init();
}

KCMultiDialog::~KCMultiDialog()
{
    delete d_ptr;
}

void KCMultiDialog::showEvent(QShowEvent *ev)
{
    KPageDialog::showEvent(ev);
    adjustSize();
    /**
     * adjustSize() relies on sizeHint but is limited to 2/3 of the desktop size
     * Workaround for https://bugreports.qt.io/browse/QTBUG-3459
     *
     * We adjust the size after passing the show event
     * because otherwise window pos is set to (0,0)
     */
    const QSize maxSize = QApplication::desktop()->availableGeometry(pos()).size();
    resize(qMin(sizeHint().width(), maxSize.width()),
           qMin(sizeHint().height(), maxSize.height()));
}

void KCMultiDialog::slotDefaultClicked()
{
    Q_D(KCMultiDialog);
    const KPageWidgetItem *item = currentPage();
    if (!item) {
        return;
    }

    for (int i = 0; i < d->modules.count(); ++i) {
        if (d->modules[ i ].item == item) {
            d->modules[ i ].kcm->defaults();
            d->_k_clientChanged();
            return;
        }
    }
}

void KCMultiDialog::slotUser1Clicked()
{
    const KPageWidgetItem *item = currentPage();
    if (!item) {
        return;
    }

    Q_D(KCMultiDialog);
    for (int i = 0; i < d->modules.count(); ++i) {
        if (d->modules[ i ].item == item) {
            d->modules[ i ].kcm->load();
            d->_k_clientChanged();
            return;
        }
    }
}

bool KCMultiDialogPrivate::moduleSave(KCModuleProxy *module)
{
    if (!module) {
        return false;
    }

    module->save();
    return true;
}

void KCMultiDialogPrivate::apply()
{
    Q_Q(KCMultiDialog);
    QStringList updatedComponents;

    foreach (const CreatedModule &module, modules) {
        KCModuleProxy *proxy = module.kcm;

        if (proxy->changed()) {
            proxy->save();
            /**
                * Add name of the components the kcm belongs to the list
                * of updated components.
                */
            const QStringList componentNames = module.componentNames;
            foreach (const QString &componentName, module.componentNames) {
                if (!updatedComponents.contains(componentName)) {
                    updatedComponents.append(componentName);
                }
            }
        }
    }

    // Send the configCommitted signal for every updated component.
    foreach (const QString &name, updatedComponents) {
        emit q->configCommitted(name.toLatin1());
    }

    emit q->configCommitted();
}

void KCMultiDialog::slotApplyClicked()
{
    QPushButton *applyButton = buttonBox()->button(QDialogButtonBox::Apply);
    applyButton->setFocus();

    d_func()->apply();
}

void KCMultiDialog::slotOkClicked()
{
    QPushButton *okButton = buttonBox()->button(QDialogButtonBox::Ok);
    okButton->setFocus();

    d_func()->apply();
    accept();
}

void KCMultiDialog::slotHelpClicked()
{
    const KPageWidgetItem *item = currentPage();
    if (!item) {
        return;
    }

    Q_D(KCMultiDialog);
    QString docPath;
    for (int i = 0; i < d->modules.count(); ++i) {
        if (d->modules[ i ].item == item) {
            docPath = d->modules[ i ].kcm->moduleInfo().docPath();
            break;
        }
    }

    const QUrl docUrl = QUrl(QStringLiteral("help:/")).resolved(QUrl(docPath)); // same code as in KHelpClient::invokeHelp
    const QString docUrlScheme = docUrl.scheme();
    if (docUrlScheme == QLatin1String("help") || docUrlScheme == QLatin1String("man") || docUrlScheme == QLatin1String("info")) {
        QProcess::startDetached(QStringLiteral("khelpcenter"), QStringList() << docUrl.toString());
    } else {
        QDesktopServices::openUrl(docUrl);
    }
}

void KCMultiDialog::closeEvent(QCloseEvent *event)
{
    Q_D(KCMultiDialog);
    KPageDialog::closeEvent(event);

    /**
     * If we don't delete them, the DBUS registration stays, and trying to load the KCMs
     * in other situations will lead to "module already loaded in Foo," while to the user
     * doesn't appear so(the dialog is hidden)
     */
    Q_FOREACH(auto &proxy, d->modules) {
        proxy.kcm->deleteClient();
    }
}

KPageWidgetItem *KCMultiDialog::addModule(const QString &path, const QStringList &args)
{
    QString complete = path;

    if (!path.endsWith(QLatin1String(".desktop"))) {
        complete += QStringLiteral(".desktop");
    }

    KService::Ptr service = KService::serviceByStorageId(complete);

    return addModule(KCModuleInfo(service), nullptr, args);
}

KPageWidgetItem *KCMultiDialog::addModule(const KCModuleInfo &moduleInfo,
        KPageWidgetItem *parentItem, const QStringList &args)
{
    if (!moduleInfo.service()) {
        return nullptr;
    }

    //KAuthorized::authorizeControlModule( moduleInfo.service()->menuId() ) is
    //checked in noDisplay already
    if (moduleInfo.service()->noDisplay()) {
        return nullptr;
    }

    // Create the scroller
    auto *moduleScroll = new UnboundScrollArea(this);
    // Prepare the scroll area
    moduleScroll->setWidgetResizable(true);
    moduleScroll->setFrameStyle(QFrame::NoFrame);
    moduleScroll->viewport()->setAutoFillBackground(false);

    KCModuleProxy *kcm = new KCModuleProxy(moduleInfo, moduleScroll, args);
    moduleScroll->setWidget(kcm);

    // qDebug() << moduleInfo.moduleName();
    KPageWidgetItem *item = new KPageWidgetItem(moduleScroll, moduleInfo.moduleName());

    if (kcm->realModule() && kcm->realModule()->useRootOnlyMessage()) {
        item->setHeader(QStringLiteral("<b>") +
                moduleInfo.comment() +
                QStringLiteral("</b><br><i>") +
                kcm->realModule()->rootOnlyMessage() +
                QStringLiteral("</i>"));
        item->setIcon(KDE::icon(moduleInfo.icon(), QStringList() << QStringLiteral("dialog-warning")));
    } else {
        item->setHeader(moduleInfo.comment());
        item->setIcon(QIcon::fromTheme(moduleInfo.icon()));
    }
    item->setProperty("_k_weight", moduleInfo.weight());

    bool updateCurrentPage = false;
    const KPageWidgetModel *model = qobject_cast<const KPageWidgetModel *>(pageWidget()->model());
    Q_ASSERT(model);
    if (parentItem) {
        const QModelIndex parentIndex = model->index(parentItem);
        const int siblingCount = model->rowCount(parentIndex);
        int row = 0;
        for (; row < siblingCount; ++row) {
            KPageWidgetItem *siblingItem = model->item(parentIndex.child(row, 0));
            if (siblingItem->property("_k_weight").toInt() > moduleInfo.weight()) {
                // the item we found is heavier than the new module
                // qDebug() << "adding KCM " << item->name() << " before " << siblingItem->name();
                insertPage(siblingItem, item);
                break;
            }
        }
        if (row >= siblingCount) {
            // the new module is either the first or the heaviest item
            // qDebug() << "adding KCM " << item->name() << " with parent " << parentItem->name();
            addSubPage(parentItem, item);
        }
    } else {
        const int siblingCount = model->rowCount();
        int row = 0;
        for (; row < siblingCount; ++row) {
            KPageWidgetItem *siblingItem = model->item(model->index(row, 0));
            if (siblingItem->property("_k_weight").toInt() > moduleInfo.weight()) {
                // the item we found is heavier than the new module
                // qDebug() << "adding KCM " << item->name() << " before " << siblingItem->name();
                insertPage(siblingItem, item);
                if (siblingItem == currentPage()) {
                    updateCurrentPage = true;
                }

                break;
            }
        }
        if (row == siblingCount) {
            // the new module is either the first or the heaviest item
            // qDebug() << "adding KCM " << item->name() << " at the top level";
            addPage(item);
        }
    }

    connect(kcm, SIGNAL(changed(bool)), this, SLOT(_k_clientChanged()));
    connect(kcm->realModule(), SIGNAL(rootOnlyMessageChanged(bool,QString)), this, SLOT(_k_updateHeader(bool,QString)));

    Q_D(KCMultiDialog);
    KCMultiDialogPrivate::CreatedModule cm;
    cm.kcm = kcm;
    cm.item = item;
    cm.componentNames = moduleInfo.service()->property(QStringLiteral("X-KDE-ParentComponents")).toStringList();
    d->modules.append(cm);

    if (d->modules.count() == 1 || updateCurrentPage) {
        setCurrentPage(item);
        d->_k_clientChanged();
    }
    return item;
}

void KCMultiDialog::clear()
{
    Q_D(KCMultiDialog);
    // qDebug() ;

    for (int i = 0; i < d->modules.count(); ++i) {
        removePage(d->modules[ i ].item);
        delete d->modules[ i ].kcm;
    }

    d->modules.clear();

    d->_k_clientChanged();
}

#include "moc_kcmultidialog.cpp"
