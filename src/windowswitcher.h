#ifndef WINDOWSWITCHER_H
#define WINDOWSWITCHER_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QAbstractListModel>

struct WindowItem {
    QString title;
    QString cls;
    QString address;
    QString workspace;
};

class WindowListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ExecRole = Qt::UserRole + 2,
        DesktopFileRole = Qt::UserRole + 3,
        GenericNameRole = Qt::UserRole + 4,
        IconNameRole = Qt::UserRole + 5
    };

    explicit WindowListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();

private:
    QList<WindowItem> m_windows;
};

class WindowSwitcher : public QObject {
    Q_OBJECT

public:
    explicit WindowSwitcher(QObject *parent = nullptr);

    Q_INVOKABLE QVariantList getOpenWindows();
    Q_INVOKABLE QVariantList getMatchingWindows(const QString &query);
    Q_INVOKABLE bool focusWindow(const QString &address);

    WindowListModel *model() { return &m_model; }

private:
    WindowListModel m_model;
};

#endif // WINDOWSWITCHER_H
