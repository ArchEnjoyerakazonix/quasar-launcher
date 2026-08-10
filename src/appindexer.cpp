#include "appindexer.h"

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QProcess>
#include <QtConcurrent/QtConcurrent>
#include <QRegularExpression>
#include <QFileInfo>

AppIndexer::AppIndexer(QObject *parent)
    : QAbstractListModel(parent)
{
    QStringList dataDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    
    QStringList extraDirs = {
        "/usr/share/applications",
        "/usr/local/share/applications",
        QDir::homePath() + "/.local/share/applications"
    };

    for (const QString& dir : extraDirs) {
        if (!dataDirs.contains(dir)) {
            dataDirs.append(dir);
        }
    }

    for (const QString& dirPath : std::as_const(dataDirs)) {
        QFileInfo fi(dirPath);
        if (fi.exists() && fi.isDir()) {
            m_scanDirs.append(dirPath);
            m_watcher.addPath(dirPath);
        }
    }

    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &AppIndexer::onFilesChanged);

    startScan();
}

AppIndexer::~AppIndexer()
{
    if (m_future.isRunning()) {
        m_future.cancel();
        m_future.waitForFinished();
    }
}

int AppIndexer::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_apps.size();
}

QVariant AppIndexer::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_apps.size()) {
        return QVariant();
    }

    const AppEntry &app = m_apps[index.row()];

    switch (role) {
    case NameRole: return app.name;
    case GenericNameRole: return app.genericName;
    case CommentRole: return app.comment;
    case ExecRole: return app.exec;
    case IconNameRole: return app.iconName;
    case CategoriesRole: return app.categories;
    case KeywordsRole: return app.keywords;
    case DesktopFileRole: return app.desktopFile;
    case ScoreRole: return app.score;
    }

    return QVariant();
}

QHash<int, QByteArray> AppIndexer::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[GenericNameRole] = "genericName";
    roles[CommentRole] = "comment";
    roles[ExecRole] = "exec";
    roles[IconNameRole] = "iconName";
    roles[CategoriesRole] = "categories";
    roles[KeywordsRole] = "keywords";
    roles[DesktopFileRole] = "desktopFile";
    roles[ScoreRole] = "score";
    return roles;
}

#include <QDesktopServices>
#include <QUrl>

void AppIndexer::launch(const QString& execStr)
{
    QString exec = execStr.trimmed();
    if (exec.isEmpty()) return;

    // 1. Web search launch
    if (exec.startsWith("__web__:") || exec == "__web__") {
        QString query = exec.startsWith("__web__:") ? exec.mid(8).trimmed() : QString();
        if (query.startsWith("?")) query = query.mid(1).trimmed();
        if (query.startsWith("g:")) query = query.mid(2).trimmed();
        if (query.startsWith("web:")) query = query.mid(4).trimmed();
        if (query.isEmpty()) return;

        QString encoded = QString::fromUtf8(QUrl::toPercentEncoding(query.toUtf8()));
        QString searchUrl = "https://www.google.com/search?q=" + encoded;
        QProcess::startDetached("xdg-open", QStringList() << searchUrl);
        return;
    }

    // 2. Shell command launch
    if (exec.startsWith("__shell__:") || exec.startsWith("$")) {
        QString cmd = exec.startsWith("__shell__:") ? exec.mid(10).trimmed() : exec.mid(1).trimmed();
        if (cmd.isEmpty()) return;
        QProcess::startDetached("kitty", QStringList() << "-1" << "bash" << "-c" << (cmd + "; read -p 'Press enter to exit...'"));
        return;
    }

    // 3. Slash action command launch
    if (exec.startsWith("/")) {
        QProcess::startDetached("bash", QStringList() << "-c" << exec);
        return;
    }

    // 4. Desktop App launch
    static const QRegularExpression fieldCodes(QStringLiteral("%[fFuUick]\\b"));
    exec.replace(fieldCodes, QString());
    exec = exec.trimmed();

    if (!exec.isEmpty()) {
        QStringList args = QProcess::splitCommand(exec);
        if (!args.isEmpty()) {
            QString program = args.takeFirst();
            QProcess::startDetached(program, args);
        }
    }
}

void AppIndexer::refresh()
{
    if (m_future.isRunning()) {
        return;
    }
    startScan();
}

int AppIndexer::filteredCount() const
{
    return m_apps.size();
}

void AppIndexer::onFilesChanged()
{
    refresh();
}

void AppIndexer::startScan()
{
    QFutureWatcher<QVector<AppEntry>> *watcher = new QFutureWatcher<QVector<AppEntry>>(this);
    connect(watcher, &QFutureWatcher<QVector<AppEntry>>::finished, this, &AppIndexer::onScanFinished);
    
    m_future = QtConcurrent::run(&AppIndexer::scanDirectories, m_scanDirs);
    watcher->setFuture(m_future);
}

void AppIndexer::onScanFinished()
{
    auto *watcher = static_cast<QFutureWatcher<QVector<AppEntry>>*>(sender());
    if (!watcher) return;

    QVector<AppEntry> newApps = watcher->result();
    watcher->deleteLater();

    beginResetModel();
    m_apps = std::move(newApps);
    endResetModel();

    qDebug() << "AppIndexer: Successfully scanned" << m_apps.size() << "applications.";

    emit filteredCountChanged();
}

QVector<AppEntry> AppIndexer::scanDirectories(const QStringList& dirs)
{
    QVector<AppEntry> entries;
    entries.reserve(512); // Pre-allocate

    QHash<QString, bool> processedIds;

    for (const QString& dirPath : dirs) {
        QDir dir(dirPath);
        if (!dir.exists()) continue;

        QFileInfoList files = dir.entryInfoList(QStringList() << "*.desktop", QDir::Files | QDir::Readable);
        for (const QFileInfo& fileInfo : std::as_const(files)) {
            QString id = fileInfo.fileName();
            if (processedIds.contains(id)) {
                continue;
            }

            AppEntry entry = parseDesktopFile(fileInfo.absoluteFilePath());
            if (!entry.id.isEmpty()) {
                entries.append(std::move(entry));
                processedIds.insert(id, true);
            }
        }
    }

    return entries;
}

AppEntry AppIndexer::parseDesktopFile(const QString& filePath)
{
    AppEntry entry;
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return entry;
    }

    QTextStream in(&file);
    bool inDesktopEntry = false;
    bool noDisplay = false;
    bool hidden = false;
    bool isApp = false;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) {
            continue;
        }

        if (line.startsWith(QLatin1Char('['))) {
            inDesktopEntry = (line == QLatin1String("[Desktop Entry]"));
            continue;
        }

        if (!inDesktopEntry) {
            continue;
        }

        int eqIndex = line.indexOf(QLatin1Char('='));
        if (eqIndex == -1) continue;

        QString key = line.left(eqIndex).trimmed();
        QString value = line.mid(eqIndex + 1).trimmed();

        if (key == QLatin1String("Type")) {
            isApp = value.startsWith(QLatin1String("Application"));
        } else if (key == QLatin1String("Name")) {
            entry.name = value;
        } else if (key.startsWith(QLatin1String("Name[")) && entry.name.isEmpty()) {
            entry.name = value;
        } else if (key == QLatin1String("GenericName") && entry.genericName.isEmpty()) {
            entry.genericName = value;
        } else if (key == QLatin1String("Comment") && entry.comment.isEmpty()) {
            entry.comment = value;
        } else if (key == QLatin1String("Exec")) {
            entry.exec = value;
        } else if (key == QLatin1String("Icon")) {
            entry.iconName = value;
        } else if (key == QLatin1String("Categories")) {
            entry.categories = value.split(QLatin1Char(';'), Qt::SkipEmptyParts);
        } else if (key == QLatin1String("Keywords") || key.startsWith(QLatin1String("Keywords["))) {
            entry.keywords.append(value.split(QLatin1Char(';'), Qt::SkipEmptyParts));
        } else if (key == QLatin1String("NoDisplay")) {
            noDisplay = (value.toLower() == QLatin1String("true"));
        } else if (key == QLatin1String("Hidden")) {
            hidden = (value.toLower() == QLatin1String("true"));
        }
    }

    if (!isApp || noDisplay || hidden || entry.exec.isEmpty()) {
        return AppEntry(); 
    }

    entry.desktopFile = filePath;
    entry.id = QFileInfo(filePath).fileName();

    return entry;
}
