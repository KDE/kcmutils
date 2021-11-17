/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007, 2006 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kpluginwidget.h"
#include "kpluginwidget_p.h"

#include <kcmutils_debug.h>

#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QStandardPaths>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <QVBoxLayout>

#include <KAboutPluginDialog>
#include <KCategorizedSortFilterProxyModel>
#include <KCategorizedView>
#include <KCategoryDrawer>
#include <KLocalizedString>
#include <KPluginMetaData>
#include <KStandardGuiItem>
#include <kcmoduleinfo.h>
#include <kcmoduleproxy.h>
#include <utility>

#define MARGIN 5

int KPluginWidgetPrivate::dependantLayoutValue(int value, int width, int totalWidth) const
{
    if (listView->layoutDirection() == Qt::LeftToRight) {
        return value;
    }

    return totalWidth - width - value;
}

KPluginWidget::KPluginWidget(QWidget *parent)
    : QWidget(parent)
    , d(new KPluginWidgetPrivate)
{
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    d->lineEdit = new QLineEdit(this);
    d->lineEdit->setClearButtonEnabled(true);
    d->lineEdit->setPlaceholderText(i18n("Search..."));
    d->listView = new KCategorizedView(this);
    d->categoryDrawer = new KCategoryDrawer(d->listView);
    d->listView->setVerticalScrollMode(QListView::ScrollPerPixel);
    d->listView->setAlternatingRowColors(true);
    d->listView->setCategoryDrawer(d->categoryDrawer);

    d->pluginModel = new KPluginModel(this);

    connect(d->pluginModel, &KPluginModel::defaulted, this, &KPluginWidget::defaulted);
    connect(d->pluginModel,
            &QAbstractItemModel::dataChanged,
            this,
            [this](const QModelIndex &topLeft, const QModelIndex & /*bottomRight*/, const QVector<int> &roles) {
                if (roles.contains(KPluginModel::EnabledRole)) {
                    Q_EMIT changed(topLeft.data(KPluginModel::IdRole).toString(), topLeft.data(KPluginModel::EnabledRole).toBool());
                }
            });

    d->proxyModel = new KPluginWidgetProxyModel(d.get(), this);
    d->proxyModel->setCategorizedModel(true);
    d->proxyModel->setSourceModel(d->pluginModel);
    d->listView->setModel(d->proxyModel /*d->pluginModel*/);
    d->listView->setAlternatingRowColors(true);

    auto pluginDelegate = new PluginDelegate(d.get(), this);
    d->listView->setItemDelegate(pluginDelegate);

    d->listView->setMouseTracking(true);
    d->listView->viewport()->setAttribute(Qt::WA_Hover);

    connect(d->lineEdit, &QLineEdit::textChanged, d->proxyModel, &QSortFilterProxyModel::invalidate);
    connect(pluginDelegate, &PluginDelegate::configCommitted, this, &KPluginWidget::configCommitted);
    connect(pluginDelegate, &PluginDelegate::changed, this, &KPluginWidget::changed);

    layout->addWidget(d->lineEdit);
    layout->addWidget(d->listView);

    // When a KPluginWidget instance gets focus,
    // it should pass over the focus to its child searchbar.
    setFocusProxy(d->lineEdit);
}

KPluginWidget::~KPluginWidget()
{
    delete d->listView->itemDelegate();
    delete d->listView; // depends on some other things in d, make sure this dies first.
}

void KPluginWidget::addPlugins(const QVector<KPluginMetaData> &plugins, const QString &categoryLabel)
{
    d->pluginModel->addPlugins(plugins, categoryLabel);
    d->proxyModel->sort(0);
}

void KPluginWidget::setConfig(const KConfigGroup &config)
{
    d->pluginModel->setConfig(config);
}

void KPluginWidget::clear()
{
    d->pluginModel->clear();
}

void KPluginWidget::save()
{
    d->pluginModel->save();
}

void KPluginWidget::defaults()
{
    d->pluginModel->defaults();
}

bool KPluginWidget::isDefault() const
{
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        const QModelIndex index = d->pluginModel->index(i, 0);
        if (d->pluginModel->data(index, Qt::CheckStateRole).toBool() != d->pluginModel->data(index, KPluginModel::EnabledByDefaultRole).toBool()) {
            return false;
        }
    }

    return true;
}

bool KPluginWidget::isSaveNeeded() const
{
    return d->pluginModel->isSaveNeeded();
}

void KPluginWidget::setConfigurationArguments(const QStringList &arguments)
{
    d->kcmArguments = arguments;
}

QStringList KPluginWidget::configurationArguments() const
{
    return d->kcmArguments;
}

void KPluginWidget::showConfiguration(const QString &pluginId)
{
    QModelIndex idx;
    for (int i = 0, c = d->proxyModel->rowCount(); i < c; ++i) {
        const auto currentIndex = d->proxyModel->index(i, 0);
        const QString id = currentIndex.data(KPluginModel::IdRole).toString();
        if (id == pluginId) {
            idx = currentIndex;
            break;
        }
    }

    if (idx.isValid()) {
        auto delegate = static_cast<PluginDelegate *>(d->listView->itemDelegate());
        delegate->configure(idx);
    } else {
        qCWarning(KCMUTILS_LOG) << "Could not find plugin" << pluginId;
    }
}

void KPluginWidget::setDefaultsIndicatorsVisible(bool isVisible)
{
    d->showDefaultIndicator = isVisible;
}

void KPluginWidget::setAdditionalButtonHandler(std::function<QPushButton *(const KPluginMetaData &)> handler)
{
    auto delegate = static_cast<PluginDelegate *>(d->listView->itemDelegate());
    delegate->handler = std::move(handler);
}

KPluginWidgetProxyModel::KPluginWidgetProxyModel(KPluginWidgetPrivate *pluginSelector_d, QObject *parent)
    : KCategorizedSortFilterProxyModel(parent)
    , pluginSelector_d(pluginSelector_d)
{
    sort(0);
}

KPluginWidgetProxyModel::~KPluginWidgetProxyModel()
{
}

bool KPluginWidgetProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)

    if (pluginSelector_d->lineEdit->text().isEmpty()) {
        return true;
    }

    const QModelIndex index = sourceModel()->index(sourceRow, 0);

    const QString name = index.data(KPluginModel::NameRole).toString();

    if (name.contains(pluginSelector_d->lineEdit->text(), Qt::CaseInsensitive)) {
        return true;
    }

    const QString description = index.data(KPluginModel::DescriptionRole).toString();

    if (description.contains(pluginSelector_d->lineEdit->text(), Qt::CaseInsensitive)) {
        return true;
    }

    return false;
}

bool KPluginWidgetProxyModel::subSortLessThan(const QModelIndex &left, const QModelIndex &right) const
{
    return left.data(KPluginModel::NameRole).toString().compare(right.data(KPluginModel::NameRole).toString()) < 0;
}

PluginDelegate::PluginDelegate(KPluginWidgetPrivate *pluginSelector_d, QObject *parent)
    : KWidgetItemDelegate(pluginSelector_d->listView, parent)
    , checkBox(new QCheckBox)
    , pushButton(new QPushButton)
    , pluginSelector_d(pluginSelector_d)
{
    pushButton->setIcon(QIcon::fromTheme(QStringLiteral("configure"))); // only for getting size matters
}

PluginDelegate::~PluginDelegate()
{
    delete checkBox;
    delete pushButton;
}

void PluginDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }

    int xOffset = checkBox->sizeHint().width();
    bool disabled = !index.model()->data(index, KPluginModel::IsChangeableRole).toBool();

    painter->save();

    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, nullptr);

    int iconSize = option.rect.height() - MARGIN * 2;
    QIcon icon = QIcon::fromTheme(index.model()->data(index, Qt::DecorationRole).toString());
    icon.paint(painter,
               QRect(pluginSelector_d->dependantLayoutValue(MARGIN + option.rect.left() + xOffset, iconSize, option.rect.width()),
                     MARGIN + option.rect.top(),
                     iconSize,
                     iconSize));

    QRect contentsRect(pluginSelector_d->dependantLayoutValue(MARGIN * 2 + iconSize + option.rect.left() + xOffset,
                                                              option.rect.width() - MARGIN * 3 - iconSize - xOffset,
                                                              option.rect.width()),
                       MARGIN + option.rect.top(),
                       option.rect.width() - MARGIN * 3 - iconSize - xOffset,
                       option.rect.height() - MARGIN * 2);

    int lessHorizontalSpace = MARGIN * 2 + pushButton->sizeHint().width();
    if (index.model()->data(index, KPluginModel::ConfigRole).value<KPluginMetaData>().isValid()) {
        lessHorizontalSpace += MARGIN + pushButton->sizeHint().width();
    }
    // Reserve space for extra button
    if (handler) {
        lessHorizontalSpace += MARGIN + pushButton->sizeHint().width();
    }

    contentsRect.setWidth(contentsRect.width() - lessHorizontalSpace);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.highlightedText().color());
    }

    if (pluginSelector_d->listView->layoutDirection() == Qt::RightToLeft) {
        contentsRect.translate(lessHorizontalSpace, 0);
    }

    painter->save();
    if (disabled) {
        QPalette pal(option.palette);
        pal.setCurrentColorGroup(QPalette::Disabled);
        painter->setPen(pal.text().color());
    }

    painter->save();
    QFont font = titleFont(option.font);
    QFontMetrics fmTitle(font);
    painter->setFont(font);
    painter->drawText(contentsRect,
                      Qt::AlignLeft | Qt::AlignTop,
                      fmTitle.elidedText(index.model()->data(index, Qt::DisplayRole).toString(), Qt::ElideRight, contentsRect.width()));
    painter->restore();

    painter->drawText(
        contentsRect,
        Qt::AlignLeft | Qt::AlignBottom,
        option.fontMetrics.elidedText(index.model()->data(index, KPluginModel::DescriptionRole).toString(), Qt::ElideRight, contentsRect.width()));

    painter->restore();
    painter->restore();
}

QSize PluginDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int i = 5;
    int j = 1;
    if (index.model()->data(index, KPluginModel::ConfigRole).value<KPluginMetaData>().isValid()) {
        i = 6;
        j = 2;
    }
    // Reserve space for extra button
    if (handler) {
        ++j;
    }

    const QFont font = titleFont(option.font);
    const QFontMetrics fmTitle(font);
    const QString text = index.model()->data(index, Qt::DisplayRole).toString();
    const QString comment = index.model()->data(index, KPluginModel::DescriptionRole).toString();
    const int maxTextWidth = qMax(fmTitle.boundingRect(text).width(), option.fontMetrics.boundingRect(comment).width());

    const auto iconSize = pluginSelector_d->listView->style()->pixelMetric(QStyle::PM_IconViewIconSize);
    return QSize(maxTextWidth + iconSize + MARGIN * i + pushButton->sizeHint().width() * j,
                 qMax(iconSize + MARGIN * 2, fmTitle.height() + option.fontMetrics.height() + MARGIN * 2));
}

QList<QWidget *> PluginDelegate::createItemWidgets(const QModelIndex &index) const
{
    Q_UNUSED(index);
    QList<QWidget *> widgetList;

    auto enabledCheckBox = new QCheckBox;
    connect(enabledCheckBox, &QAbstractButton::clicked, this, &PluginDelegate::slotStateChanged);

    auto aboutPushButton = new QPushButton;
    aboutPushButton->setIcon(QIcon::fromTheme(QStringLiteral("dialog-information")));
    aboutPushButton->setToolTip(i18n("About"));
    connect(aboutPushButton, &QAbstractButton::clicked, this, &PluginDelegate::slotAboutClicked);

    auto configurePushButton = new QPushButton;
    configurePushButton->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    configurePushButton->setToolTip(i18n("Configure"));
    connect(configurePushButton, &QAbstractButton::clicked, this, &PluginDelegate::slotConfigureClicked);

    setBlockedEventTypes(enabledCheckBox,
                         QList<QEvent::Type>() << QEvent::MouseButtonPress << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick << QEvent::KeyPress
                                               << QEvent::KeyRelease);

    setBlockedEventTypes(aboutPushButton,
                         QList<QEvent::Type>() << QEvent::MouseButtonPress << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick << QEvent::KeyPress
                                               << QEvent::KeyRelease);

    setBlockedEventTypes(configurePushButton,
                         QList<QEvent::Type>() << QEvent::MouseButtonPress << QEvent::MouseButtonRelease << QEvent::MouseButtonDblClick << QEvent::KeyPress
                                               << QEvent::KeyRelease);

    widgetList << enabledCheckBox << configurePushButton << aboutPushButton;
    if (handler) {
        QPushButton *btn = handler(pluginSelector_d->pluginModel->data(index, KPluginModel::MetaDataRole).value<KPluginMetaData>());
        if (btn) {
            widgetList << btn;
        }
    }

    return widgetList;
}

void PluginDelegate::updateItemWidgets(const QList<QWidget *> widgets, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const
{
    int extraButtonWidth = 0;
    QPushButton *extraButton = nullptr;
    if (widgets.count() == 4) {
        extraButton = static_cast<QPushButton *>(widgets[3]);
        extraButtonWidth = extraButton->sizeHint().width() + MARGIN;
    }
    auto checkBox = static_cast<QCheckBox *>(widgets[0]);
    checkBox->resize(checkBox->sizeHint());
    checkBox->move(pluginSelector_d->dependantLayoutValue(MARGIN, checkBox->sizeHint().width(), option.rect.width()),
                   option.rect.height() / 2 - checkBox->sizeHint().height() / 2);

    auto aboutPushButton = static_cast<QPushButton *>(widgets[2]);
    QSize aboutPushButtonSizeHint = aboutPushButton->sizeHint();
    aboutPushButton->resize(aboutPushButtonSizeHint);
    aboutPushButton->move(pluginSelector_d->dependantLayoutValue(option.rect.width() - MARGIN - aboutPushButtonSizeHint.width() - extraButtonWidth,
                                                                 aboutPushButtonSizeHint.width(),
                                                                 option.rect.width()),
                          option.rect.height() / 2 - aboutPushButtonSizeHint.height() / 2);

    auto configurePushButton = static_cast<QPushButton *>(widgets[1]);
    QSize configurePushButtonSizeHint = configurePushButton->sizeHint();
    configurePushButton->resize(configurePushButtonSizeHint);
    configurePushButton->move(pluginSelector_d->dependantLayoutValue(option.rect.width() - MARGIN * 2 - configurePushButtonSizeHint.width()
                                                                         - aboutPushButtonSizeHint.width() - extraButtonWidth,
                                                                     configurePushButtonSizeHint.width(),
                                                                     option.rect.width()),
                              option.rect.height() / 2 - configurePushButtonSizeHint.height() / 2);
    if (extraButton) {
        QSize extraPushButtonSizeHint = extraButton->sizeHint();
        extraButton->resize(extraPushButtonSizeHint);
        extraButton->move(pluginSelector_d->dependantLayoutValue(option.rect.width() - extraButtonWidth, extraPushButtonSizeHint.width(), option.rect.width()),
                          option.rect.height() / 2 - extraPushButtonSizeHint.height() / 2);
    }

    if (!index.isValid() || !index.internalPointer()) {
        checkBox->setVisible(false);
        aboutPushButton->setVisible(false);
        configurePushButton->setVisible(false);
        if (extraButton) {
            extraButton->setVisible(false);
        }
    } else {
        bool enabledByDefault = pluginSelector_d->pluginModel->data(index, KPluginModel::EnabledByDefaultRole).toBool();
        bool enabled = pluginSelector_d->pluginModel->data(index, KPluginModel::EnabledRole).toBool();
        checkBox->setProperty("_kde_highlight_neutral", pluginSelector_d->showDefaultIndicator && enabledByDefault != enabled);
        checkBox->setChecked(index.model()->data(index, Qt::CheckStateRole).toBool());
        checkBox->setEnabled(index.model()->data(index, KPluginModel::IsChangeableRole).toBool());
        configurePushButton->setVisible(index.model()->data(index, KPluginModel::ConfigRole).value<KPluginMetaData>().isValid());
        configurePushButton->setEnabled(index.model()->data(index, Qt::CheckStateRole).toBool());
    }
}

void PluginDelegate::slotStateChanged(bool state)
{
    if (!focusedIndex().isValid()) {
        return;
    }

    QModelIndex index = focusedIndex();

    const_cast<QAbstractItemModel *>(index.model())->setData(index, state, Qt::CheckStateRole);
}

void PluginDelegate::slotAboutClicked()
{
    const QModelIndex index = focusedIndex();

    auto pluginMetaData = index.data(KPluginModel::MetaDataRole).value<KPluginMetaData>();

    auto *aboutPlugin = new KAboutPluginDialog(pluginMetaData, itemView());
    aboutPlugin->setAttribute(Qt::WA_DeleteOnClose);
    aboutPlugin->show();
}

void PluginDelegate::slotConfigureClicked()
{
    configure(focusedIndex());
}

void PluginDelegate::configure(const QModelIndex &index)
{
    const QAbstractItemModel *model = index.model();
    const auto kcm = model->data(index, KPluginModel::ConfigRole).value<KPluginMetaData>();

    QDialog configDialog(itemView());
    configDialog.setWindowTitle(model->data(index, KPluginModel::NameRole).toString());

    auto moduleProxy = new KCModuleProxy(kcm, &configDialog, pluginSelector_d->kcmArguments);

    if (!moduleProxy->realModule()) {
        delete moduleProxy;
        return;
    }

    auto layout = new QVBoxLayout(&configDialog);
    layout->addWidget(moduleProxy);

    auto buttonBox = new QDialogButtonBox(&configDialog);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::RestoreDefaults), KStandardGuiItem::defaults());
    connect(buttonBox, &QDialogButtonBox::accepted, &configDialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &configDialog, &QDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, [moduleProxy] {
        moduleProxy->defaults();
    });
    layout->addWidget(buttonBox);

    if (configDialog.exec() == QDialog::Accepted) {
        Q_EMIT configCommitted(model->data(index, KPluginModel::IdRole).toString());
        moduleProxy->save();
    } else {
        moduleProxy->load();
    }
}

QFont PluginDelegate::titleFont(const QFont &baseFont) const
{
    QFont retFont(baseFont);
    retFont.setBold(true);

    return retFont;
}
