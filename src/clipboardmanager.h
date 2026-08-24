#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QString>
#include <QClipboard>

struct ClipboardItem {
    QString id;
    QString preview;
    QString fullText;
    bool isColor = false;
    QString colorHex;
};

class ClipboardManager : public QAbstractListModel {
    Q_OBJECT

public:
    enum ClipboardRoles {
        NameRole = Qt::UserRole + 1,
        GenericNameRole = Qt::UserRole + 2,
        CommentRole = Qt::UserRole + 3,
        ExecRole = Qt::UserRole + 4,
        IconNameRole = Qt::UserRole + 5,
        CategoriesRole = Qt::UserRole + 6,
        KeywordsRole = Qt::UserRole + 7,
        DesktopFileRole = Qt::UserRole + 8
    };

    explicit ClipboardManager(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();
    void restoreItem(int index);

private slots:
    void onSystemClipboardChanged();

private:
    void fetchFromCliphist();
    QVector<ClipboardItem> m_items;
};
