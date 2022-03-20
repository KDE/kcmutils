#include "kcmutilscore_export.h"

#include <KCategorizedSortFilterProxyModel>

class Q_DECL_HIDDEN KPluginProxyModel : public KCategorizedSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
public:
    explicit KPluginProxyModel(QObject *parent = nullptr);
    ~KPluginProxyModel() override;

    QString query() const;
    void setQuery(const QString &query);

Q_SIGNALS:
    void queryChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool subSortLessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    QString m_query;
};
