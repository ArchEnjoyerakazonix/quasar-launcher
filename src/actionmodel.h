#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>

struct ActionItem {
    QString name;
    QString command;
    QString iconName;
    QString description;
};

class ActionModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ExecRole,
        IconNameRole,
        CommentRole,
        CategoryRole,
        DesktopFileRole
    };

    explicit ActionModel(QObject *parent = nullptr);
    ~ActionModel() override = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void execute(const QString &command);

private:
    QVector<ActionItem> m_actions;
};
