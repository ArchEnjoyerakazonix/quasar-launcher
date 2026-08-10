#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QFuture>

struct AppEntry {
    QString id;
    QString name;
    QString genericName;
    QString comment;
    QString exec;
    QString iconName;
    QStringList categories;
    QStringList keywords;
    QString desktopFile;
    int score = 0;
};

class AppIndexer : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int filteredCount READ filteredCount NOTIFY filteredCountChanged)

public:
    enum AppRoles {
        NameRole = Qt::UserRole + 1,
        GenericNameRole,
        CommentRole,
        ExecRole,
        IconNameRole,
        CategoriesRole,
        KeywordsRole,
        DesktopFileRole,
        ScoreRole
    };

    explicit AppIndexer(QObject *parent = nullptr);
    ~AppIndexer() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void launch(const QString& exec);
    Q_INVOKABLE void refresh();

    int filteredCount() const;

signals:
    void filteredCountChanged();

private slots:
    void onFilesChanged();
    void onScanFinished();

private:
    void startScan();
    static QVector<AppEntry> scanDirectories(const QStringList& dirs);
    static AppEntry parseDesktopFile(const QString& filePath);

    QVector<AppEntry> m_apps;
    QFileSystemWatcher m_watcher;
    QFuture<QVector<AppEntry>> m_future;
    QStringList m_scanDirs;
};
