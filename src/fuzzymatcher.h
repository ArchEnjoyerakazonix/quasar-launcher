#pragma once

#include <QSortFilterProxyModel>
#include <QString>
#include <QHash>
#include <unordered_map>

class FuzzyMatcher : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)

public:
    explicit FuzzyMatcher(QObject *parent = nullptr);
    ~FuzzyMatcher() override = default;

    QString query() const;
    void setQuery(const QString &newQuery);

    void setSourceModel(QAbstractItemModel *sourceModel) override;
    void setWindowModel(QAbstractItemModel *windowModel);
    void setAppIndexerModel(QAbstractItemModel *appModel);
    void setActionModel(QAbstractItemModel *actionModel);
    
    enum CustomRoles {
        ScoreRole = Qt::UserRole + 150,
        HighlightedNameRole = Qt::UserRole + 151
    };

    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void queryChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    QString m_query;
    QAbstractItemModel *m_appIndexerModel = nullptr;
    QAbstractItemModel *m_windowModel = nullptr;
    QAbstractItemModel *m_actionModel = nullptr;
    mutable std::unordered_map<int, int> m_scoreCache;

    int m_nameRole = -1;
    int m_genericNameRole = -1;
    int m_commentRole = -1;
    int m_execRole = -1;
    int m_categoriesRole = -1;
    int m_keywordsRole = -1;
    int m_desktopFileRole = -1;
    bool m_corpusHasCyrillic = false;

    void updateRoleKeys();
    int score(int sourceRow) const;
    QString formatHighlightedName(const QString &name, const QString &query) const;
};
