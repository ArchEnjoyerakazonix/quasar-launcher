#include "frecencyranker.h"
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace {
    const qint64 ONE_HOUR = 3600;
    const qint64 ONE_DAY = 86400;
    const qint64 SEVEN_DAYS = 604800;
    const qint64 THIRTY_DAYS = 2592000;
    const qint64 NINETY_DAYS = 7776000;
}

FrecencyRanker *FrecencyRanker::instance()
{
    static FrecencyRanker s_instance;
    return &s_instance;
}

FrecencyRanker::FrecencyRanker(QObject *parent)
    : QObject(parent)
    , m_saveTimer(new QTimer(this))
{
    m_saveTimer->setSingleShot(true);
    m_saveTimer->setInterval(5000); // 5-second debounce
    connect(m_saveTimer, &QTimer::timeout, this, &FrecencyRanker::save);

    load();
}

FrecencyRanker::~FrecencyRanker()
{
    if (m_saveTimer->isActive()) {
        m_saveTimer->stop();
        save();
    }
}

int FrecencyRanker::totalLaunches() const
{
    return m_totalLaunches;
}

void FrecencyRanker::recordLaunch(const QString &appId)
{
    qint64 now = QDateTime::currentSecsSinceEpoch();
    m_launches[appId].append(now);
    m_totalLaunches++;
    emit totalLaunchesChanged();

    scheduleSave();
}

double FrecencyRanker::getScore(const QString &appId) const
{
    if (!m_launches.contains(appId)) {
        return 0.0;
    }

    qint64 now = QDateTime::currentSecsSinceEpoch();
    double score = 0.0;

    for (qint64 launchTime : m_launches.value(appId)) {
        qint64 age = now - launchTime;
        if (age < 0) age = 0; // Guard against future timestamps

        if (age <= ONE_HOUR) {
            score += 4.0;
        } else if (age <= ONE_DAY) {
            score += 2.0;
        } else if (age <= SEVEN_DAYS) {
            score += 1.0;
        } else if (age <= THIRTY_DAYS) {
            score += 0.5;
        } else {
            score += 0.25;
        }
    }

    return score;
}

QString FrecencyRanker::saveFilePath() const
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/quasar";
    return configDir + "/frecency.json";
}

void FrecencyRanker::scheduleSave()
{
    m_saveTimer->start();
}

void FrecencyRanker::load()
{
    QString filePath = saveFilePath();
    QFile file(filePath);
    
    m_totalLaunches = 0;
    
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            QJsonObject root = doc.object();
            for (auto it = root.begin(); it != root.end(); ++it) {
                QString appId = it.key();
                QJsonArray times = it.value().toArray();
                QList<qint64> launchTimes;
                for (const QJsonValue &v : times) {
                    launchTimes.append(v.toVariant().toLongLong());
                }
                m_launches[appId] = launchTimes;
                m_totalLaunches += launchTimes.size();
            }
        }
        file.close();
    }
    
    prune();
}

void FrecencyRanker::save()
{
    QString filePath = saveFilePath();
    QFileInfo fileInfo(filePath);
    QDir().mkpath(fileInfo.absolutePath());
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonObject root;
        for (auto it = m_launches.constBegin(); it != m_launches.constEnd(); ++it) {
            QJsonArray times;
            for (qint64 t : it.value()) {
                times.append(t);
            }
            root[it.key()] = times;
        }
        
        QJsonDocument doc(root);
        file.write(doc.toJson());
        file.close();
    }
}

void FrecencyRanker::prune()
{
    qint64 cutoff = QDateTime::currentSecsSinceEpoch() - NINETY_DAYS;
    bool changed = false;
    
    auto it = m_launches.begin();
    while (it != m_launches.end()) {
        QList<qint64> &times = it.value();
        int originalSize = times.size();
        
        times.erase(std::remove_if(times.begin(), times.end(),
            [cutoff](qint64 t) { return t < cutoff; }), times.end());
            
        if (times.isEmpty()) {
            it = m_launches.erase(it);
            changed = true;
        } else {
            if (times.size() != originalSize) {
                changed = true;
            }
            ++it;
        }
    }
    
    if (changed) {
        m_totalLaunches = 0;
        for (const auto &times : m_launches) {
            m_totalLaunches += times.size();
        }
        emit totalLaunchesChanged();
        scheduleSave();
    }
}
