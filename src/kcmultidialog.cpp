/*
    SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
    SPDX-FileCopyrightText: 2003 Daniel Molkentin <molkentin@kde.org>
    SPDX-FileCopyrightText: 2003, 2006 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcmultidialog.h"
#include "kcmultidialog_p.h"

#include "kcmoduleproxy.h"
#include "kcmoduleqml_p.h"
#include <kcmutils_debug.h>

#include <QApplication>
#include <QDesktopServices>
#include <QJsonArray>
#include <QLayout>
#include <QProcess>
#include <QPushButton>
#include <QScreen>
#include <QStandardPaths>
#include <QStringList>
#include <QStyle>
#include <QUrl>

#if KCONFIGWIDGETS_WITH_KAUTH
#include <KAuth/Action>
#include <KAuth/ObjectDecorator>
#endif
#include <KGuiItem>
#include <KIconUtils>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPageWidgetModel>
#include <KPluginInfo>

bool KCMultiDialogPrivate::resolveChanges(KCModuleProxy *currentProxy)
{
    Q_Q(KCMultiDialog);
    if (!currentProxy || !currentProxy->isChanged()) {
        return true;
    }

    // Let the user decide
    const int queryUser = KMessageBox::warningTwoActionsCancel(q,
                                                               i18n("The settings of the current module have changed.\n"
                                                                    "Do you want to apply the changes or discard them?"),
                                                               i18n("Apply Settings"),
                                                               KStandardGuiItem::apply(),
                                                               KStandardGuiItem::discard(),
                                                               KStandardGuiItem::cancel());

    switch (queryUser) {
    case KMessageBox::PrimaryAction:
        return moduleSave(currentProxy);

    case KMessageBox::SecondaryAction:
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

    KCModuleProxy *previousModule = nullptr;
    for (int i = 0; i < modules.count(); ++i) {
        if (modules[i].item == previous) {
            previousModule = modules[i].kcm;
        }
    }

    // Delete global margins and spacing, since we want the contents to
    // be able to touch the edges of the window
    q->layout()->setContentsMargins(0, 0, 0, 0);

    const KPageWidget *pageWidget = q->pageWidget();
    pageWidget->layout()->setSpacing(0);

    // Then, we set the margins for the title header and the buttonBox footer
    const QStyle *style = q->style();
    const QMargins layoutMargins = QMargins(style->pixelMetric(QStyle::PM_LayoutLeftMargin),
                                            style->pixelMetric(QStyle::PM_LayoutTopMargin),
                                            style->pixelMetric(QStyle::PM_LayoutRightMargin),
                                            style->pixelMetric(QStyle::PM_LayoutBottomMargin));

    if (pageWidget->pageHeader()) {
        pageWidget->pageHeader()->setContentsMargins(layoutMargins);
    }

    // Do not set buttonBox's top margin as that space will be covered by the content's bottom margin
    q->buttonBox()->setContentsMargins(layoutMargins.left(), 0, layoutMargins.right(), layoutMargins.bottom());

    q->blockSignals(true);
    q->setCurrentPage(previous);

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
        if (modules[i].item == q->currentPage()) {
            activeModule = modules[i].kcm;
            break;
        }
    }

    bool change = false;
    bool defaulted = false;
    if (activeModule) {
        change = activeModule->isChanged();
        defaulted = activeModule->defaulted();

        QPushButton *applyButton = q->buttonBox()->button(QDialogButtonBox::Apply);
        if (applyButton) {
            q->disconnect(applyButton, &QAbstractButton::clicked, q, &KCMultiDialog::slotApplyClicked);
#if KCONFIGWIDGETS_WITH_KAUTH
            delete applyButton->findChild<KAuth::ObjectDecorator *>();
#endif
        }

        QPushButton *okButton = q->buttonBox()->button(QDialogButtonBox::Ok);
        if (okButton) {
            q->disconnect(okButton, &QAbstractButton::clicked, q, &KCMultiDialog::slotOkClicked);
#if KCONFIGWIDGETS_WITH_KAUTH
            delete okButton->findChild<KAuth::ObjectDecorator *>();
#endif
        }

#if KCONFIGWIDGETS_WITH_KAUTH
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
#endif
    }

    auto buttons = activeModule ? activeModule->buttons() : KCModule::NoAdditionalButton;

    QPushButton *resetButton = q->buttonBox()->button(QDialogButtonBox::Reset);
    if (resetButton) {
        resetButton->setVisible(buttons & KCModule::Apply);
        resetButton->setEnabled(change);
    }

    QPushButton *applyButton = q->buttonBox()->button(QDialogButtonBox::Apply);
    if (applyButton) {
        applyButton->setVisible(buttons & KCModule::Apply);
        applyButton->setEnabled(change);
    }

    QPushButton *cancelButton = q->buttonBox()->button(QDialogButtonBox::Cancel);
    if (cancelButton) {
        cancelButton->setVisible(buttons & KCModule::Apply);
    }

    QPushButton *okButton = q->buttonBox()->button(QDialogButtonBox::Ok);
    if (okButton) {
        okButton->setVisible(buttons & KCModule::Apply);
    }

    QPushButton *closeButton = q->buttonBox()->button(QDialogButtonBox::Close);
    if (closeButton) {
        closeButton->setHidden(buttons & KCModule::Apply);
    }

    QPushButton *helpButton = q->buttonBox()->button(QDialogButtonBox::Help);
    if (helpButton) {
        helpButton->setVisible(buttons & KCModule::Help);
    }

    QPushButton *defaultButton = q->buttonBox()->button(QDialogButtonBox::RestoreDefaults);
    if (defaultButton) {
        defaultButton->setVisible(buttons & KCModule::Default);
        defaultButton->setEnabled(!defaulted);
    }
}

void KCMultiDialogPrivate::_k_updateHeader(bool use, const QString &message)
{
    Q_Q(KCMultiDialog);
    KPageWidgetItem *item = q->currentPage();
    KCModuleProxy *kcm = qobject_cast<KCModuleProxy *>(item->widget());

    QString moduleName;
    QString icon;

    if (kcm->metaData().isValid()) {
        moduleName = kcm->metaData().name();
        icon = kcm->metaData().iconName();
    }

#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 88)
    if (kcm->moduleInfo().isValid()) {
        moduleName = kcm->moduleInfo().moduleName();
        icon = kcm->moduleInfo().icon();
    }
#endif

    if (use) {
        item->setHeader(QStringLiteral("<b>") + moduleName + QStringLiteral("</b><br><i>") + message + QStringLiteral("</i>"));
        item->setIcon(KIconUtils::addOverlay(QIcon::fromTheme(icon), QIcon::fromTheme(QStringLiteral("dialog-warning")), Qt::BottomRightCorner));
    } else {
        item->setHeader(moduleName);
        item->setIcon(QIcon::fromTheme(icon));
    }
}

void KCMultiDialogPrivate::init()
{
    Q_Q(KCMultiDialog);
    q->setFaceType(KPageDialog::Auto);
    q->setWindowTitle(i18n("Configure"));
    q->setModal(false);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(q);
    buttonBox->setStandardButtons(QDialogButtonBox::Help | QDialogButtonBox::RestoreDefaults | QDialogButtonBox::Cancel | QDialogButtonBox::Apply
                                  | QDialogButtonBox::Close | QDialogButtonBox::Ok | QDialogButtonBox::Reset);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::RestoreDefaults), KStandardGuiItem::defaults());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Apply), KStandardGuiItem::apply());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Close), KStandardGuiItem::close());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Reset), KStandardGuiItem::reset());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Help), KStandardGuiItem::help());
    buttonBox->button(QDialogButtonBox::Close)->setVisible(false);
    buttonBox->button(QDialogButtonBox::Reset)->setEnabled(false);
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

    q->connect(buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, q, &KCMultiDialog::slotApplyClicked);
    q->connect(buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::clicked, q, &KCMultiDialog::slotOkClicked);
    q->connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, q, &KCMultiDialog::slotDefaultClicked);
    q->connect(buttonBox->button(QDialogButtonBox::Help), &QAbstractButton::clicked, q, &KCMultiDialog::slotHelpClicked);
    q->connect(buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, q, &KCMultiDialog::slotUser1Clicked);

    q->setButtonBox(buttonBox);
    q->connect(q, &KPageDialog::currentPageChanged, q, [this](KPageWidgetItem *current, KPageWidgetItem *before) {
        _k_slotCurrentPageChanged(current, before);
    });
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

    const QSize maxSize = screen()->availableGeometry().size();
    resize(qMin(sizeHint().width(), maxSize.width()), qMin(sizeHint().height(), maxSize.height()));
}

void KCMultiDialog::slotDefaultClicked()
{
    Q_D(KCMultiDialog);
    const KPageWidgetItem *item = currentPage();
    if (!item) {
        return;
    }

    for (int i = 0; i < d->modules.count(); ++i) {
        if (d->modules[i].item == item) {
            d->modules[i].kcm->defaults();
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
        if (d->modules[i].item == item) {
            d->modules[i].kcm->load();
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

    for (const CreatedModule &module : std::as_const(modules)) {
        KCModuleProxy *proxy = module.kcm;

        if (proxy->isChanged()) {
            proxy->save();
#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 85)
            /**
             * Add name of the components the kcm belongs to the list
             * of updated components.
             */
            const QStringList componentNames = module.componentNames;
            for (const QString &componentName : componentNames) {
                if (!updatedComponents.contains(componentName)) {
                    updatedComponents.append(componentName);
                }
            }
#endif
        }
    }

#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 85)
    // Send the configCommitted signal for every updated component.
    for (const QString &name : std::as_const(updatedComponents)) {
        Q_EMIT q->configCommitted(name.toLatin1());
    }
#endif

    Q_EMIT q->configCommitted();
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
        if (d->modules[i].item == item) {
#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 88)
            docPath = d->modules[i].kcm->moduleInfo().docPath();
#endif
            if (docPath.isEmpty()) {
                docPath = d->modules[i].kcm->metaData().value(QStringLiteral("X-DocPath"));
            }
            break;
        }
    }

    const QUrl docUrl = QUrl(QStringLiteral("help:/")).resolved(QUrl(docPath)); // same code as in KHelpClient::invokeHelp
    const QString docUrlScheme = docUrl.scheme();
    const QString helpExec = QStandardPaths::findExecutable(QStringLiteral("khelpcenter"));
    const bool foundExec = !helpExec.isEmpty();
    if (!foundExec) {
        qCDebug(KCMUTILS_LOG) << "Couldn't find khelpcenter executable in PATH.";
    }
    if (foundExec && (docUrlScheme == QLatin1String("man") || docUrlScheme == QLatin1String("info"))) {
        QProcess::startDetached(helpExec, QStringList() << docUrl.toString());
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
    for (auto &proxy : std::as_const(d->modules)) {
        proxy.kcm->deleteClient();
    }
}

KPageWidgetItem *KCMultiDialog::addModule(const KPluginMetaData &metaData)
{
    return addModule(metaData, QStringList());
}

KPageWidgetItem *KCMultiDialog::addModule(const KPluginMetaData &metaData, const QStringList &args)
{
    Q_D(KCMultiDialog);
    // Create the scroller
    auto *moduleScroll = new UnboundScrollArea(this);
    // Prepare the scroll area
    moduleScroll->setWidgetResizable(true);
    moduleScroll->setFrameStyle(QFrame::NoFrame);
    moduleScroll->viewport()->setAutoFillBackground(false);

    KCModuleProxy *kcm = new KCModuleProxy(metaData, moduleScroll, args);
    moduleScroll->setWidget(kcm);

    KPageWidgetItem *item = new KPageWidgetItem(moduleScroll, metaData.name());

    KCMultiDialogPrivate::CreatedModule createdModule;
    createdModule.kcm = kcm;
    createdModule.item = item;
    d->modules.append(createdModule);

    if (qobject_cast<KCModuleQml *>(kcm->realModule())) {
        item->setHeaderVisible(false);
    }

    if (kcm->realModule() && kcm->realModule()->useRootOnlyMessage()) {
        item->setHeader(QStringLiteral("<b>%1</b><br><i>%2</i>").arg(metaData.name(), kcm->realModule()->rootOnlyMessage()));
        item->setIcon(KIconUtils::addOverlay(QIcon::fromTheme(metaData.iconName()), QIcon::fromTheme(QStringLiteral("dialog-warning")), Qt::BottomRightCorner));
    } else {
        item->setHeader(metaData.name());
        item->setIcon(QIcon::fromTheme(metaData.iconName()));
    }
    const int weight = metaData.rawData().value(QStringLiteral("X-KDE-Weight")).toInt();
    item->setProperty("_k_weight", weight);

    bool updateCurrentPage = false;
    const KPageWidgetModel *model = qobject_cast<const KPageWidgetModel *>(pageWidget()->model());
    Q_ASSERT(model);
    const int siblingCount = model->rowCount();
    int row = 0;
    for (; row < siblingCount; ++row) {
        KPageWidgetItem *siblingItem = model->item(model->index(row, 0));
        if (siblingItem->property("_k_weight").toInt() > weight) {
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

    QObject::connect(kcm, qOverload<bool>(&KCModuleProxy::changed), this, [d]() {
        d->_k_clientChanged();
    });

    QObject::connect(kcm->realModule(), &KCModule::rootOnlyMessageChanged, this, [d](bool use, const QString &message) {
        d->_k_updateHeader(use, message);
    });

    if (d->modules.count() == 1 || updateCurrentPage) {
        setCurrentPage(item);
        d->_k_clientChanged();
    }
    return item;
}

#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 85)
KPageWidgetItem *KCMultiDialog::addModule(const QString &path, const QStringList &args)
{
    QString complete = path;

    if (!path.endsWith(QLatin1String(".desktop"))) {
        complete += QStringLiteral(".desktop");
    }

    KService::Ptr service = KService::serviceByStorageId(complete);

    return addModule(KCModuleInfo(service), nullptr, args);
}
#endif

#if KCMUTILS_BUILD_DEPRECATED_SINCE(5, 85)
KPageWidgetItem *KCMultiDialog::addModule(const KCModuleInfo &moduleInfo, KPageWidgetItem *parentItem, const QStringList &args)
{
    Q_D(KCMultiDialog);
    if (!moduleInfo.isValid()) {
        return nullptr;
    }

    // KAuthorized::authorizeControlModule( moduleInfo.service()->menuId() ) is
    // checked in noDisplay already
    if (moduleInfo.service() && moduleInfo.service()->noDisplay()) {
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

    KCMultiDialogPrivate::CreatedModule cm;
    cm.kcm = kcm;
    cm.item = item;
    cm.componentNames = moduleInfo.property(QStringLiteral("X-KDE-ParentComponents")).toStringList();
    d->modules.append(cm);

    if (qobject_cast<KCModuleQml *>(kcm->realModule())) {
        item->setHeaderVisible(false);
    }

    if (kcm->realModule() && kcm->realModule()->useRootOnlyMessage()) {
        item->setHeader(QStringLiteral("<b>") + moduleInfo.moduleName() + QStringLiteral("</b><br><i>") + kcm->realModule()->rootOnlyMessage()
                        + QStringLiteral("</i>"));
        item->setIcon(KIconUtils::addOverlay(QIcon::fromTheme(moduleInfo.icon()), QIcon::fromTheme(QStringLiteral("dialog-warning")), Qt::BottomRightCorner));
    } else {
        item->setHeader(moduleInfo.moduleName());
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
            KPageWidgetItem *siblingItem = model->item(model->index(row, 0, parentIndex));
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

    QObject::connect(kcm, qOverload<bool>(&KCModuleProxy::changed), this, [d]() {
        d->_k_clientChanged();
    });

    QObject::connect(kcm->realModule(), &KCModule::rootOnlyMessageChanged, this, [d](bool use, QString message) {
        d->_k_updateHeader(use, message);
    });

    if (d->modules.count() == 1 || updateCurrentPage) {
        setCurrentPage(item);
        d->_k_clientChanged();
    }
    return item;
}
#endif

void KCMultiDialog::clear()
{
    Q_D(KCMultiDialog);
    // qDebug() ;

    for (int i = 0; i < d->modules.count(); ++i) {
        removePage(d->modules[i].item);
    }

    d->modules.clear();

    d->_k_clientChanged();
}

#include "moc_kcmultidialog.cpp"
