#include "kcmutilscore_export.h"

#include <KCategorizedSortFilterProxyModel>
#include <KPluginMetaData>

class Q_DECL_HIDDEN KPluginProxyModel : public KCategorizedSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(QAbstractListModel *model WRITE setModel)
    Q_PROPERTY(KPluginMetaData dummyMetaData READ dummyMetaData CONSTANT)
public:
    explicit KPluginProxyModel(QObject *parent = nullptr);
    ~KPluginProxyModel() override;

    QString query() const;
    void setQuery(const QString &query);
    void setModel(QAbstractListModel *model)
    {
        setSourceModel(model);
    }

    KPluginMetaData dummyMetaData()
    {
        return KPluginMetaData{};
    }

Q_SIGNALS:
    void queryChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool subSortLessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    QString m_query;
};
