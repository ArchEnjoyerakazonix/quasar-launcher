#ifndef WINDOWSWITCHER_H
#define WINDOWSWITCHER_H

#include <QObject>
#include <QString>
#include <QList>
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
        GenericNameRole = Qt::UserRole + 2,
        CommentRole = Qt::UserRole + 3,
        ExecRole = Qt::UserRole + 4,
        IconNameRole = Qt::UserRole + 5,
        CategoriesRole = Qt::UserRole + 6,
        KeywordsRole = Qt::UserRole + 7,
        DesktopFileRole = Qt::UserRole + 8
    };

    explicit WindowListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void refresh();

private:
    void applyWindows(QList<WindowItem> windows);

    static QList<WindowItem> parseHyprlandClients(const QByteArray &output);
    static QList<WindowItem> parseSwayTree(const QByteArray &output);
    static QList<WindowItem> parseWmctrlList(const QByteArray &output);

    QList<WindowItem> m_windows;
    bool m_refreshPending = false;
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
