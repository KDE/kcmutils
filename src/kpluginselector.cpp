/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007, 2006 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kpluginselector.h"
#include "kpluginselector_p.h"

#include <kcmutils_debug.h>

#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDirIterator>
#include <QLabel>
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
#include <KMessageBox>
#include <KPluginMetaData>
#include <KStandardGuiItem>
#include <KUrlLabel>
#include <kcmoduleinfo.h>
#include <kcmoduleproxy.h>

#define MARGIN 5

KPluginSelector::Private::Private(KPluginSelector *parent)
    : QObject(parent)
    , parent(parent)
    , listView(nullptr)
    , categoryDrawer(nullptr)
    , pluginDelegate(nullptr)
    , showIcons(false)
    , showDefaultIndicator(false)
{
}

KPluginSelector::Private::~Private()
{
}

void KPluginSelector::Private::updateDependencies(PluginEntry *pluginEntry, bool added)
{
    if (added) {
        QStringList dependencyList = pluginEntry->pluginInfo.dependencies();

        if (dependencyList.isEmpty()) {
            return;
        }

        for (int i = 0; i < pluginModel->rowCount(); i++) {
            const QModelIndex index = pluginModel->index(i, 0);
            PluginEntry *pe = static_cast<PluginEntry *>(index.internalPointer());

            if ((pe->pluginInfo.pluginName() != pluginEntry->pluginInfo.pluginName()) && dependencyList.contains(pe->pluginInfo.pluginName()) && !pe->checked) {
                dependenciesWidget->addDependency(pe->pluginInfo.name(), pluginEntry->pluginInfo.name(), added);
                const_cast<QAbstractItemModel *>(index.model())->setData(index, added, Qt::CheckStateRole);
                updateDependencies(pe, added);
            }
        }
    } else {
        for (int i = 0; i < pluginModel->rowCount(); i++) {
            const QModelIndex index = pluginModel->index(i, 0);
            PluginEntry *pe = static_cast<PluginEntry *>(index.internalPointer());

            if ((pe->pluginInfo.pluginName() != pluginEntry->pluginInfo.pluginName())
                && pe->pluginInfo.dependencies().contains(pluginEntry->pluginInfo.pluginName()) && pe->checked) {
                dependenciesWidget->addDependency(pe->pluginInfo.name(), pluginEntry->pluginInfo.name(), added);
                const_cast<QAbstractItemModel *>(index.model())->setData(index, added, Qt::CheckStateRole);
                updateDependencies(pe, added);
            }
        }
    }
}

int KPluginSelector::Private::dependantLayoutValue(int value, int width, int totalWidth) const
{
    if (listView->layoutDirection() == Qt::LeftToRight) {
        return value;
    }

    return totalWidth - width - value;
}

KPluginSelector::Private::DependenciesWidget::DependenciesWidget(QWidget *parent)
    : QWidget(parent)
    , addedByDependencies(0)
    , removedByDependencies(0)
{
    setVisible(false);

    details = new QLabel();

    QHBoxLayout *layout = new QHBoxLayout(this);

    QVBoxLayout *dataLayout = new QVBoxLayout;
    dataLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    layout->setAlignment(Qt::AlignLeft);
    QLabel *label = new QLabel();
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-information")).pixmap(style()->pixelMetric(QStyle::PM_MessageBoxIconSize)));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addWidget(label);
    KUrlLabel *link = new KUrlLabel();
    link->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    link->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    link->setGlowEnabled(false);
    link->setUnderline(false);
    link->setFloatEnabled(true);
    link->setUseCursor(true);
    link->setHighlightedColor(palette().color(QPalette::Link));
    link->setSelectedColor(palette().color(QPalette::Link));
    link->setText(i18n("Automatic changes have been performed due to plugin dependencies. Click here for further information"));
    dataLayout->addWidget(link);
    dataLayout->addWidget(details);
    layout->addLayout(dataLayout);

    QObject::connect(link, &KUrlLabel::leftClickedUrl, this, &KPluginSelector::Private::DependenciesWidget::showDependencyDetails);
}

KPluginSelector::Private::DependenciesWidget::~DependenciesWidget()
{
}

void KPluginSelector::Private::DependenciesWidget::addDependency(const QString &dependency, const QString &pluginCausant, bool added)
{
    if (!isVisible()) {
        setVisible(true);
    }

    struct FurtherInfo furtherInfo;
    furtherInfo.added = added;
    furtherInfo.pluginCausant = pluginCausant;

    if (dependencyMap.contains(dependency)) { // The dependency moved from added to removed or vice-versa
        if (added && removedByDependencies) {
            removedByDependencies--;
        } else if (addedByDependencies) {
            addedByDependencies--;
        }

        dependencyMap[dependency] = furtherInfo;
    } else {
        dependencyMap.insert(dependency, furtherInfo);
    }

    if (added) {
        addedByDependencies++;
    } else {
        removedByDependencies++;
    }

    updateDetails();
}

void KPluginSelector::Private::DependenciesWidget::userOverrideDependency(const QString &dependency)
{
    if (dependencyMap.contains(dependency)) {
        if (addedByDependencies && dependencyMap[dependency].added) {
            addedByDependencies--;
        } else if (removedByDependencies) {
            removedByDependencies--;
        }

        dependencyMap.remove(dependency);
    }

    updateDetails();
}

void KPluginSelector::Private::DependenciesWidget::clearDependencies()
{
    addedByDependencies = 0;
    removedByDependencies = 0;
    dependencyMap.clear();
    updateDetails();
}

void KPluginSelector::Private::DependenciesWidget::showDependencyDetails()
{
    QString message = i18n("Automatic changes have been performed in order to satisfy plugin dependencies:\n");

    for (auto it = dependencyMap.cbegin(); it != dependencyMap.cend(); ++it) {
        const QString &dependency = it.key();
        const FurtherInfo &info = it.value();
        if (info.added) {
            message += i18n("\n    %1 plugin has been automatically checked because of the dependency of %2 plugin", dependency, info.pluginCausant);
        } else {
            message += i18n("\n    %1 plugin has been automatically unchecked because of its dependency on %2 plugin", dependency, info.pluginCausant);
        }
    }

    KMessageBox::information(this, message, i18n("Dependency Check"));

    addedByDependencies = 0;
    removedByDependencies = 0;
    updateDetails();
}

void KPluginSelector::Private::DependenciesWidget::updateDetails()
{
    if (dependencyMap.isEmpty()) {
        setVisible(false);
        return;
    }

    QString message;

    if (addedByDependencies) {
        message +=
            i18np("%1 plugin automatically added due to plugin dependencies", "%1 plugins automatically added due to plugin dependencies", addedByDependencies);
    }

    if (removedByDependencies && !message.isEmpty()) {
        message += i18n(", ");
    }

    if (removedByDependencies) {
        message += i18np("%1 plugin automatically removed due to plugin dependencies",
                         "%1 plugins automatically removed due to plugin dependencies",
                         removedByDependencies);
    }

    if (message.isEmpty()) {
        details->setVisible(false);
    } else {
        details->setVisible(true);
        details->setText(message);
    }
}

KPluginSelector::KPluginSelector(QWidget *parent)
    : QWidget(parent)
    , d(new Private(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    d->lineEdit = new QLineEdit(this);
    d->lineEdit->setClearButtonEnabled(true);
    d->lineEdit->setPlaceholderText(i18n("Search..."));
    d->listView = new KCategorizedView(this);
    d->categoryDrawer = new KCategoryDrawer(d->listView);
    d->listView->setVerticalScrollMode(QListView::ScrollPerPixel);
    d->listView->setAlternatingRowColors(true);
    d->listView->setCategoryDrawer(d->categoryDrawer);
    d->dependenciesWidget = new Private::DependenciesWidget(this);

    d->pluginModel = new Private::PluginModel(d, this);
    d->proxyModel = new Private::ProxyModel(d, this);
    d->proxyModel->setCategorizedModel(true);
    d->proxyModel->setSourceModel(d->pluginModel);
    d->listView->setModel(d->proxyModel);
    d->listView->setAlternatingRowColors(true);

    Private::PluginDelegate *pluginDelegate = new Private::PluginDelegate(d, this);
    d->listView->setItemDelegate(pluginDelegate);

    d->listView->setMouseTracking(true);
    d->listView->viewport()->setAttribute(Qt::WA_Hover);

    connect(d->lineEdit, &QLineEdit::textChanged, d->proxyModel, &QSortFilterProxyModel::invalidate);
    connect(pluginDelegate, &Private::PluginDelegate::changed, this, &KPluginSelector::changed);
    connect(pluginDelegate, &Private::PluginDelegate::configCommitted, this, &KPluginSelector::configCommitted);
    connect(this, &KPluginSelector::defaultsIndicatorsVisible, pluginDelegate, &Private::PluginDelegate::slotResetModel);

    connect(this, &KPluginSelector::changed, [this] {
        Q_EMIT defaulted(isDefault());
    });

    layout->addWidget(d->lineEdit);
    layout->addWidget(d->listView);
    layout->addWidget(d->dependenciesWidget);

    // When a KPluginSelector instance gets focus,
    // it should pass over the focus to its child searchbar.
    setFocusProxy(d->lineEdit);
}

KPluginSelector::~KPluginSelector()
{
    delete d->listView->itemDelegate();
    delete d->listView; // depends on some other things in d, make sure this dies first.
    delete d;
}

void KPluginSelector::addPlugins(const QString &componentName, const QString &categoryName, const QString &categoryKey, KSharedConfig::Ptr config)
{
    QStringList desktopFileNames;
    const QStringList dirs =
        QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, componentName + QStringLiteral("/kpartplugins"), QStandardPaths::LocateDirectory);
    for (const QString &dir : dirs) {
        QDirIterator it(dir, QStringList() << QStringLiteral("*.desktop"), QDir::NoFilter, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            desktopFileNames.append(it.next());
        }
    }

    QList<KPluginInfo> pluginInfoList = KPluginInfo::fromFiles(desktopFileNames);

    if (pluginInfoList.isEmpty()) {
        return;
    }

    if (!config) {
        config = KSharedConfig::openConfig(componentName + QStringLiteral("rc"));
    }
    Q_ASSERT(config);

    KConfigGroup cfgGroup(config, "KParts Plugins");
    // qDebug() << "cfgGroup = " << &cfgGroup;

    d->pluginModel->addPlugins(pluginInfoList, categoryName, categoryKey, cfgGroup);
    d->proxyModel->sort(0);
}

void KPluginSelector::addPlugins(const QList<KPluginInfo> &pluginInfoList,
                                 PluginLoadMethod pluginLoadMethod,
                                 const QString &categoryName,
                                 const QString &categoryKey,
                                 const KSharedConfig::Ptr &config)
{
    if (pluginInfoList.isEmpty()) {
        return;
    }

    KConfigGroup cfgGroup(config ? config : KSharedConfig::openConfig(), "Plugins");
    // qDebug() << "cfgGroup = " << &cfgGroup;

    d->pluginModel->addPlugins(pluginInfoList, categoryName, categoryKey, cfgGroup, pluginLoadMethod, true /* manually added */);
    d->proxyModel->sort(0);
}

void KPluginSelector::clearPlugins()
{
    d->pluginModel->clear();
}

void KPluginSelector::load()
{
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        const QModelIndex index = d->pluginModel->index(i, 0);
        PluginEntry *pluginEntry = static_cast<PluginEntry *>(index.internalPointer());
        pluginEntry->pluginInfo.load(pluginEntry->cfgGroup);
        d->pluginModel->setData(index, pluginEntry->pluginInfo.isPluginEnabled(), Qt::CheckStateRole);
    }

    static_cast<KPluginSelector::Private::PluginDelegate *>(d->listView->itemDelegate())->clearChangedEntries();
    Q_EMIT changed(false);
}

void KPluginSelector::save()
{
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        const QModelIndex index = d->pluginModel->index(i, 0);
        PluginEntry *pluginEntry = static_cast<PluginEntry *>(index.internalPointer());
        pluginEntry->pluginInfo.setPluginEnabled(pluginEntry->checked);
        pluginEntry->pluginInfo.save(pluginEntry->cfgGroup);
        pluginEntry->cfgGroup.sync();
    }

    static_cast<KPluginSelector::Private::PluginDelegate *>(d->listView->itemDelegate())->clearChangedEntries();
    Q_EMIT changed(false);
}

bool KPluginSelector::isSaveNeeded() const
{
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        const QModelIndex index = d->pluginModel->index(i, 0);
        PluginEntry *pluginEntry = static_cast<PluginEntry *>(index.internalPointer());
        if (d->pluginModel->data(index, Qt::CheckStateRole).toBool() != pluginEntry->pluginInfo.isPluginEnabled()) {
            return true;
        }
    }

    return false;
}

void KPluginSelector::defaults()
{
    bool isChanged = false;
    auto delegate = static_cast<KPluginSelector::Private::PluginDelegate *>(d->listView->itemDelegate());
    delegate->clearChangedEntries();
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        const QModelIndex index = d->pluginModel->index(i, 0);
        PluginEntry *pluginEntry = static_cast<PluginEntry *>(index.internalPointer());
        bool entryChanged = pluginEntry->pluginInfo.isPluginEnabled() != pluginEntry->pluginInfo.isPluginEnabledByDefault();
        isChanged |= entryChanged;
        d->pluginModel->setData(index, pluginEntry->pluginInfo.isPluginEnabledByDefault(), Qt::CheckStateRole);
        if (entryChanged) {
            delegate->addChangedEntry(pluginEntry);
        }
    }

    Q_EMIT changed(isChanged);
}

bool KPluginSelector::isDefault() const
{
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        const QModelIndex index = d->pluginModel->index(i, 0);
        PluginEntry *pluginEntry = static_cast<PluginEntry *>(index.internalPointer());
        if (d->pluginModel->data(index, Qt::CheckStateRole).toBool() != pluginEntry->pluginInfo.isPluginEnabledByDefault()) {
            return false;
        }
    }

    return true;
}

void KPluginSelector::updatePluginsState()
{
    static_cast<KPluginSelector::Private::PluginDelegate *>(d->listView->itemDelegate())->clearChangedEntries();
    for (int i = 0; i < d->pluginModel->rowCount(); i++) {
        const QModelIndex index = d->pluginModel->index(i, 0);
        PluginEntry *pluginEntry = static_cast<PluginEntry *>(index.internalPointer());
        if (pluginEntry->manuallyAdded) {
            pluginEntry->pluginInfo.setPluginEnabled(pluginEntry->checked);
        }
    }
}

void KPluginSelector::setConfigurationArguments(const QStringList &arguments)
{
    d->kcmArguments = arguments;
}

QStringList KPluginSelector::configurationArguments() const
{
    return d->kcmArguments;
}

void KPluginSelector::showConfiguration(const QString &componentName)
{
    QModelIndex idx;
    for (int i = 0, c = d->proxyModel->rowCount(); i < c; ++i) {
        const auto currentIndex = d->proxyModel->index(i, 0);
        const auto entry = currentIndex.data(KPluginSelector::Private::PluginEntryRole).value<PluginEntry *>();
        if (entry->pluginInfo.pluginName() == componentName) {
            idx = currentIndex;
            break;
        }
    }

    if (idx.isValid()) {
        auto delegate = static_cast<KPluginSelector::Private::PluginDelegate *>(d->listView->itemDelegate());
        delegate->configure(idx);
    } else {
        qCWarning(KCMUTILS_LOG) << "Could not find plugin" << componentName;
    }
}

void KPluginSelector::setAdditionalButtonHandler(std::function<QPushButton *(const KPluginInfo &)> handler)
{
    static_cast<Private::PluginDelegate *>(d->listView->itemDelegate())->setHandler(handler);
}

void KPluginSelector::setDefaultsIndicatorsVisible(bool isVisible)
{
    if (isVisible != d->showDefaultIndicator) {
        d->showDefaultIndicator = isVisible;
        Q_EMIT defaultsIndicatorsVisible();
    }
}

KPluginSelector::Private::PluginModel::PluginModel(KPluginSelector::Private *pluginSelector_d, QObject *parent)
    : QAbstractListModel(parent)
    , pluginSelector_d(pluginSelector_d)
{
}

KPluginSelector::Private::PluginModel::~PluginModel()
{
}

static bool hasServiceNoDisplaySet(const KPluginInfo &pluginInfo)
{
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
    QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
    return pluginInfo.service() && pluginInfo.service()->noDisplay();
    QT_WARNING_POP
}

void KPluginSelector::Private::PluginModel::addPlugins(const QList<KPluginInfo> &pluginList,
                                                       const QString &categoryName,
                                                       const QString &categoryKey,
                                                       const KConfigGroup &cfgGroup,
                                                       PluginLoadMethod pluginLoadMethod,
                                                       bool manuallyAdded)
{
    QList<PluginEntry> listToAdd;

    for (const KPluginInfo &pluginInfo : pluginList) {
        PluginEntry pluginEntry;
        pluginEntry.category = categoryName;
        pluginEntry.pluginInfo = pluginInfo;
        if (pluginLoadMethod == ReadConfigFile) {
            pluginEntry.pluginInfo.load(cfgGroup);
        }
        pluginEntry.checked = pluginInfo.isPluginEnabled();
        pluginEntry.manuallyAdded = manuallyAdded;
        if (cfgGroup.isValid()) {
            pluginEntry.cfgGroup = cfgGroup;
        } else {
            pluginEntry.cfgGroup = pluginInfo.config();
        }

        // this is where kiosk will set if a plugin is checkable or not (pluginName + "Enabled")
        pluginEntry.isCheckable = !pluginInfo.isValid() || !pluginEntry.cfgGroup.isEntryImmutable(pluginInfo.pluginName() + QLatin1String("Enabled"));

        if (!pluginEntryList.contains(pluginEntry) && !listToAdd.contains(pluginEntry)
            && (categoryKey.isEmpty() || !pluginInfo.category().compare(categoryKey, Qt::CaseInsensitive)) && (!hasServiceNoDisplaySet(pluginInfo))) {
            listToAdd << pluginEntry;

            if (!pluginSelector_d->showIcons && !pluginInfo.icon().isEmpty()) {
                pluginSelector_d->showIcons = true;
            }
        }
    }

    if (!listToAdd.isEmpty()) {
        beginInsertRows(QModelIndex(), pluginEntryList.count(), pluginEntryList.count() + listToAdd.count() - 1);
        pluginEntryList << listToAdd;
        endInsertRows();
    }
}

void KPluginSelector::Private::PluginModel::clear()
{
    beginResetModel();
    pluginEntryList.clear();
    endResetModel();
}

QList<KService::Ptr> KPluginSelector::Private::PluginModel::pluginServices(const QModelIndex &index) const
{
    return static_cast<PluginEntry *>(index.internalPointer())->pluginInfo.kcmServices();
}

QModelIndex KPluginSelector::Private::PluginModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return createIndex(row, column, (row < pluginEntryList.count()) ? (void *)&pluginEntryList.at(row) : nullptr);
}

QVariant KPluginSelector::Private::PluginModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !index.internalPointer()) {
        return QVariant();
    }

    PluginEntry *pluginEntry = static_cast<PluginEntry *>(index.internalPointer());

    switch (role) {
    case Qt::DisplayRole:
        return pluginEntry->pluginInfo.name();
    case PluginEntryRole:
        return QVariant::fromValue(pluginEntry);
    case ServicesCountRole:
        return pluginEntry->pluginInfo.kcmServices().count();
    case NameRole:
        return pluginEntry->pluginInfo.name();
    case CommentRole:
        return pluginEntry->pluginInfo.comment();
    case AuthorRole:
        return pluginEntry->pluginInfo.author();
    case EmailRole:
        return pluginEntry->pluginInfo.email();
    case WebsiteRole:
        return pluginEntry->pluginInfo.website();
    case VersionRole:
        return pluginEntry->pluginInfo.version();
    case LicenseRole:
        return pluginEntry->pluginInfo.license();
    case DependenciesRole:
        return pluginEntry->pluginInfo.dependencies();
    case IsCheckableRole:
        return pluginEntry->isCheckable;
    case Qt::DecorationRole:
        return pluginEntry->pluginInfo.icon();
    case Qt::CheckStateRole:
        return pluginEntry->checked;
    case KCategorizedSortFilterProxyModel::CategoryDisplayRole: // fall through
    case KCategorizedSortFilterProxyModel::CategorySortRole:
        return pluginEntry->category;
    default:
        return QVariant();
    }
}

bool KPluginSelector::Private::PluginModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    bool ret = false;

    if (role == Qt::CheckStateRole) {
        static_cast<PluginEntry *>(index.internalPointer())->checked = value.toBool();
        ret = true;
    }

    if (ret) {
        Q_EMIT dataChanged(index, index);
    }

    return ret;
}

int KPluginSelector::Private::PluginModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return pluginEntryList.count();
}

KPluginSelector::Private::ProxyModel::ProxyModel(KPluginSelector::Private *pluginSelector_d, QObject *parent)
    : KCategorizedSortFilterProxyModel(parent)
    , pluginSelector_d(pluginSelector_d)
{
    sort(0);
}

KPluginSelector::Private::ProxyModel::~ProxyModel()
{
}

bool KPluginSelector::Private::ProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)

    if (!pluginSelector_d->lineEdit->text().isEmpty()) {
        const QModelIndex index = sourceModel()->index(sourceRow, 0);
        const KPluginInfo pluginInfo = static_cast<PluginEntry *>(index.internalPointer())->pluginInfo;
        return pluginInfo.name().contains(pluginSelector_d->lineEdit->text(), Qt::CaseInsensitive)
            || pluginInfo.comment().contains(pluginSelector_d->lineEdit->text(), Qt::CaseInsensitive);
    }

    return true;
}

bool KPluginSelector::Private::ProxyModel::subSortLessThan(const QModelIndex &left, const QModelIndex &right) const
{
    return static_cast<PluginEntry *>(left.internalPointer())
               ->pluginInfo.name()
               .compare(static_cast<PluginEntry *>(right.internalPointer())->pluginInfo.name(), Qt::CaseInsensitive)
        < 0;
}

KPluginSelector::Private::PluginDelegate::PluginDelegate(KPluginSelector::Private *pluginSelector_d, QObject *parent)
    : KWidgetItemDelegate(pluginSelector_d->listView, parent)
    , checkBox(new QCheckBox)
    , pushButton(new QPushButton)
    , pluginSelector_d(pluginSelector_d)
{
    pushButton->setIcon(QIcon::fromTheme(QStringLiteral("configure"))); // only for getting size matters
}

KPluginSelector::Private::PluginDelegate::~PluginDelegate()
{
    delete checkBox;
    delete pushButton;
}

void KPluginSelector::Private::PluginDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }

    int xOffset = checkBox->sizeHint().width();
    bool disabled = !index.model()->data(index, IsCheckableRole).toBool();

    painter->save();

    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, nullptr);

    int iconSize = option.rect.height() - MARGIN * 2;
    if (pluginSelector_d->showIcons) {
        QIcon icon = QIcon::fromTheme(index.model()->data(index, Qt::DecorationRole).toString());
        icon.paint(painter,
                   QRect(pluginSelector_d->dependantLayoutValue(MARGIN + option.rect.left() + xOffset, iconSize, option.rect.width()),
                         MARGIN + option.rect.top(),
                         iconSize,
                         iconSize));
    } else {
        iconSize = -MARGIN;
    }

    QRect contentsRect(pluginSelector_d->dependantLayoutValue(MARGIN * 2 + iconSize + option.rect.left() + xOffset,
                                                              option.rect.width() - MARGIN * 3 - iconSize - xOffset,
                                                              option.rect.width()),
                       MARGIN + option.rect.top(),
                       option.rect.width() - MARGIN * 3 - iconSize - xOffset,
                       option.rect.height() - MARGIN * 2);

    int lessHorizontalSpace = MARGIN * 2 + pushButton->sizeHint().width();
    if (index.model()->data(index, ServicesCountRole).toBool()) {
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

    painter->drawText(contentsRect,
                      Qt::AlignLeft | Qt::AlignBottom,
                      option.fontMetrics.elidedText(index.model()->data(index, CommentRole).toString(), Qt::ElideRight, contentsRect.width()));

    painter->restore();
    painter->restore();
}

QSize KPluginSelector::Private::PluginDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int i = 5;
    int j = 1;
    if (index.model()->data(index, ServicesCountRole).toBool()) {
        i = 6;
        j = 2;
    }
    // Reserve space for extra button
    if (handler) {
        ++j;
    }

    if (!pluginSelector_d->showIcons) {
        i--;
    }

    const QFont font = titleFont(option.font);
    const QFontMetrics fmTitle(font);
    const QString text = index.model()->data(index, Qt::DisplayRole).toString();
    const QString comment = index.model()->data(index, CommentRole).toString();
    const int maxTextWidth = qMax(fmTitle.boundingRect(text).width(), option.fontMetrics.boundingRect(comment).width());

    const auto iconSize = pluginSelector_d->listView->style()->pixelMetric(QStyle::PM_IconViewIconSize);
    return QSize(maxTextWidth + (pluginSelector_d->showIcons ? iconSize : 0) + MARGIN * i + pushButton->sizeHint().width() * j,
                 qMax(iconSize + MARGIN * 2, fmTitle.height() + option.fontMetrics.height() + MARGIN * 2));
}

QList<QWidget *> KPluginSelector::Private::PluginDelegate::createItemWidgets(const QModelIndex &index) const
{
    Q_UNUSED(index);
    QList<QWidget *> widgetList;

    QCheckBox *enabledCheckBox = new QCheckBox;
    connect(enabledCheckBox, &QAbstractButton::clicked, this, &PluginDelegate::slotStateChanged);
    connect(enabledCheckBox, &QAbstractButton::clicked, this, &PluginDelegate::emitChanged);

    QPushButton *aboutPushButton = new QPushButton;
    aboutPushButton->setIcon(QIcon::fromTheme(QStringLiteral("dialog-information")));
    aboutPushButton->setToolTip(i18n("About"));
    connect(aboutPushButton, &QAbstractButton::clicked, this, &PluginDelegate::slotAboutClicked);

    QPushButton *configurePushButton = new QPushButton;
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
        QPushButton *btn = handler(pluginSelector_d->pluginModel->pluginEntryList.at(index.row()).pluginInfo);
        if (btn) {
            widgetList << btn;
        }
    }

    return widgetList;
}

void KPluginSelector::Private::PluginDelegate::updateItemWidgets(const QList<QWidget *> widgets,
                                                                 const QStyleOptionViewItem &option,
                                                                 const QPersistentModelIndex &index) const
{
    int extraButtonWidth = 0;
    QPushButton *extraButton = nullptr;
    if (widgets.count() == 4) {
        extraButton = static_cast<QPushButton *>(widgets[3]);
        extraButtonWidth = extraButton->sizeHint().width() + MARGIN;
    }
    QCheckBox *checkBox = static_cast<QCheckBox *>(widgets[0]);
    checkBox->resize(checkBox->sizeHint());
    checkBox->move(pluginSelector_d->dependantLayoutValue(MARGIN, checkBox->sizeHint().width(), option.rect.width()),
                   option.rect.height() / 2 - checkBox->sizeHint().height() / 2);

    QPushButton *aboutPushButton = static_cast<QPushButton *>(widgets[2]);
    QSize aboutPushButtonSizeHint = aboutPushButton->sizeHint();
    aboutPushButton->resize(aboutPushButtonSizeHint);
    aboutPushButton->move(pluginSelector_d->dependantLayoutValue(option.rect.width() - MARGIN - aboutPushButtonSizeHint.width() - extraButtonWidth,
                                                                 aboutPushButtonSizeHint.width(),
                                                                 option.rect.width()),
                          option.rect.height() / 2 - aboutPushButtonSizeHint.height() / 2);

    QPushButton *configurePushButton = static_cast<QPushButton *>(widgets[1]);
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
        PluginEntry *pluginEntry = index.model()->data(index, PluginEntryRole).value<PluginEntry *>();
        bool isDefault = pluginEntry->pluginInfo.isPluginEnabledByDefault() == index.model()->data(index, Qt::CheckStateRole).toBool();
        checkBox->setProperty("_kde_highlight_neutral", pluginSelector_d->showDefaultIndicator && !isDefault);

        checkBox->setChecked(index.model()->data(index, Qt::CheckStateRole).toBool());
        checkBox->setEnabled(index.model()->data(index, IsCheckableRole).toBool());
        configurePushButton->setVisible(index.model()->data(index, ServicesCountRole).toBool());
        configurePushButton->setEnabled(index.model()->data(index, Qt::CheckStateRole).toBool());
    }
}

void KPluginSelector::Private::PluginDelegate::slotStateChanged(bool state)
{
    if (!focusedIndex().isValid()) {
        return;
    }

    const QModelIndex index = focusedIndex();

    pluginSelector_d->dependenciesWidget->clearDependencies();

    PluginEntry *pluginEntry = index.model()->data(index, PluginEntryRole).value<PluginEntry *>();
    pluginSelector_d->updateDependencies(pluginEntry, state);

    const_cast<QAbstractItemModel *>(index.model())->setData(index, state, Qt::CheckStateRole);
}

void KPluginSelector::Private::PluginDelegate::emitChanged(bool state)
{
    const QModelIndex index = focusedIndex();
    PluginEntry *pluginEntry = index.model()->data(index, PluginEntryRole).value<PluginEntry *>();

    if (pluginEntry->pluginInfo.isPluginEnabled() != state) {
        changedEntries << pluginEntry;
    } else {
        changedEntries.remove(pluginEntry);
    }
    Q_EMIT changed(!changedEntries.isEmpty());
}

void KPluginSelector::Private::PluginDelegate::slotAboutClicked()
{
    const QModelIndex index = focusedIndex();
    const QAbstractItemModel *model = index.model();

    PluginEntry *pluginEntry = model->data(index, PluginEntryRole).value<PluginEntry *>();
    KPluginMetaData pluginMetaData = pluginEntry->pluginInfo.toMetaData();

    auto *aboutPlugin = new KAboutPluginDialog(pluginMetaData, itemView());
    aboutPlugin->setAttribute(Qt::WA_DeleteOnClose);
    aboutPlugin->show();
}

void KPluginSelector::Private::PluginDelegate::slotConfigureClicked()
{
    configure(focusedIndex());
}

void KPluginSelector::Private::PluginDelegate::configure(const QModelIndex &index)
{
    const QAbstractItemModel *model = index.model();

    PluginEntry *pluginEntry = model->data(index, PluginEntryRole).value<PluginEntry *>();
    KPluginInfo pluginInfo = pluginEntry->pluginInfo;

    QDialog configDialog(itemView());
    configDialog.setWindowTitle(model->data(index, NameRole).toString());
    // The number of KCModuleProxies in use determines whether to use a tabwidget
    QTabWidget *newTabWidget = nullptr;
    // Widget to use for the setting dialog's main widget,
    // either a QTabWidget or a KCModuleProxy
    QWidget *mainWidget = nullptr;
    // Widget to use as the KCModuleProxy's parent.
    // The first proxy is owned by the dialog itself
    QWidget *moduleProxyParentWidget = &configDialog;

    const auto lstServices = pluginInfo.kcmServices();
    for (const KService::Ptr &servicePtr : lstServices) {
        if (!servicePtr->noDisplay()) {
            KCModuleInfo moduleInfo(servicePtr);
            KCModuleProxy *currentModuleProxy = new KCModuleProxy(moduleInfo, moduleProxyParentWidget, pluginSelector_d->kcmArguments);
            if (currentModuleProxy->realModule()) {
                moduleProxyList << currentModuleProxy;
                if (mainWidget && !newTabWidget) {
                    // we already created one KCModuleProxy, so we need a tab widget.
                    // Move the first proxy into the tab widget and ensure this and subsequent
                    // proxies are in the tab widget
                    newTabWidget = new QTabWidget(&configDialog);
                    moduleProxyParentWidget = newTabWidget;
                    mainWidget->setParent(newTabWidget);
                    KCModuleProxy *moduleProxy = qobject_cast<KCModuleProxy *>(mainWidget);
                    if (moduleProxy) {
                        newTabWidget->addTab(mainWidget, moduleProxy->moduleInfo().moduleName());
                        mainWidget = newTabWidget;
                    } else {
                        delete newTabWidget;
                        newTabWidget = nullptr;
                        moduleProxyParentWidget = &configDialog;
                        mainWidget->setParent(nullptr);
                    }
                }

                if (newTabWidget) {
                    newTabWidget->addTab(currentModuleProxy, servicePtr->name());
                } else {
                    mainWidget = currentModuleProxy;
                }
            } else {
                delete currentModuleProxy;
            }
        }
    }

    // it could happen that we had services to show, but none of them were real modules.
    if (!moduleProxyList.isEmpty()) {
        QVBoxLayout *layout = new QVBoxLayout(&configDialog);
        layout->addWidget(mainWidget);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(&configDialog);
        buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
        KGuiItem::assign(buttonBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
        KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
        KGuiItem::assign(buttonBox->button(QDialogButtonBox::RestoreDefaults), KStandardGuiItem::defaults());
        connect(buttonBox, &QDialogButtonBox::accepted, &configDialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &configDialog, &QDialog::reject);
        connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, this, &PluginDelegate::slotDefaultClicked);
        layout->addWidget(buttonBox);

        if (configDialog.exec() == QDialog::Accepted) {
            for (KCModuleProxy *moduleProxy : qAsConst(moduleProxyList)) {
                const QStringList parentComponents = moduleProxy->moduleInfo().property(QStringLiteral("X-KDE-ParentComponents")).toStringList();
                moduleProxy->save();
                for (const QString &parentComponent : parentComponents) {
                    Q_EMIT configCommitted(parentComponent.toLatin1());
                }
            }
        } else {
            for (KCModuleProxy *moduleProxy : qAsConst(moduleProxyList)) {
                moduleProxy->load();
            }
        }

        qDeleteAll(moduleProxyList);
        moduleProxyList.clear();
    }
}

void KPluginSelector::Private::PluginDelegate::slotDefaultClicked()
{
    for (KCModuleProxy *moduleProxy : qAsConst(moduleProxyList)) {
        moduleProxy->defaults();
    }
}

void KPluginSelector::Private::PluginDelegate::slotResetModel()
{
    resetModel();
}

QFont KPluginSelector::Private::PluginDelegate::titleFont(const QFont &baseFont) const
{
    QFont retFont(baseFont);
    retFont.setBold(true);

    return retFont;
}
void KPluginSelector::Private::PluginDelegate::setHandler(std::function<QPushButton *(const KPluginInfo &)> handler)
{
    this->handler = handler;
}

#include "moc_kpluginselector.cpp"
#include "moc_kpluginselector_p.cpp"
