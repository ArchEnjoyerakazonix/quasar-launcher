#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QString>
#include <QStringList>

struct EmojiItem {
    QString emoji;
    QString name;
    QString category;
    QStringList keywords;
};

class EmojiManager : public QAbstractListModel {
    Q_OBJECT

public:
    enum EmojiRoles {
        NameRole = Qt::UserRole + 1,
        GenericNameRole = Qt::UserRole + 2,
        CommentRole = Qt::UserRole + 3,
        ExecRole = Qt::UserRole + 4,
        IconNameRole = Qt::UserRole + 5,
        CategoriesRole = Qt::UserRole + 6,
        KeywordsRole = Qt::UserRole + 7,
        DesktopFileRole = Qt::UserRole + 8
    };

    explicit EmojiManager(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    const QVector<EmojiItem>& items() const { return m_items; }

private:
    void loadEmojiDatabase();
    QVector<EmojiItem> m_items;
};
