#pragma once

#include <QObject>
#include <QString>
#include <QHash>
#include <QList>
#include <QTimer>

class FrecencyRanker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int totalLaunches READ totalLaunches NOTIFY totalLaunchesChanged)

public:
    static FrecencyRanker *instance();

    Q_INVOKABLE void recordLaunch(const QString &appId);
    Q_INVOKABLE double getScore(const QString &appId) const;

    int totalLaunches() const;

signals:
    void totalLaunchesChanged();

private:
    explicit FrecencyRanker(QObject *parent = nullptr);
    ~FrecencyRanker() override;

    void load();
    void save();
    void prune();
    void scheduleSave();
    QString saveFilePath() const;

    QHash<QString, QList<qint64>> m_launches;
    QTimer *m_saveTimer;
    int m_totalLaunches = 0;
};
