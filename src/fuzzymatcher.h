#pragma once

#include <QSortFilterProxyModel>
#include <QString>
#include <QHash>
#include <QObject>
#include <QVector>
#include <QTimer>
#include <unordered_map>
#include "calculator.h"

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
    void setEmojiModel(QAbstractItemModel *emojiModel);
    void setClipboardModel(QAbstractItemModel *clipboardModel);

    enum CustomRoles {
        ScoreRole = Qt::UserRole + 150,
        HighlightedNameRole = Qt::UserRole + 151
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
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
    QAbstractItemModel *m_emojiModel = nullptr;
    QAbstractItemModel *m_clipboardModel = nullptr;
    QTimer m_windowRefreshDebounceTimer;
    mutable std::unordered_map<int, int> m_scoreCache;
    std::optional<QString> m_mathResult;
    Calculator m_calculator;

    // Pipe plugin mode: while a pipe action (e.g. "/windows") is active the
    // source is the pipe result model and rows are filtered by the remainder
    // of the query after the action name (unless the action consumes the
    // remainder itself via "input": "query").
    bool m_pipeMode = false;
    QString m_pipeActionName;
    QString m_pipeRemainder;
    bool m_pipeFilterEnabled = true;

    int m_nameRole = -1;
    int m_genericNameRole = -1;
    int m_commentRole = -1;
    int m_execRole = -1;
    int m_iconNameRole = -1;
    int m_categoriesRole = -1;
    int m_keywordsRole = -1;
    int m_desktopFileRole = -1;
    bool m_corpusHasCyrillic = false;
    QVector<QMetaObject::Connection> m_sourceConnections;

    void updateRoleKeys();
    int score(int sourceRow) const;
    QString formatHighlightedName(const QString &name, const QString &query) const;
};
