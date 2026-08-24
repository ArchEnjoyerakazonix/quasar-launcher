#include "emojimanager.h"
#include "logging.h"
#include "platform.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCoreApplication>
#include <QDir>

EmojiManager::EmojiManager(QObject *parent)
    : QAbstractListModel(parent)
{
    loadEmojiDatabase();
}

int EmojiManager::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_items.size();
}

QVariant EmojiManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    const EmojiItem &item = m_items.at(index.row());
    switch (role) {
    case NameRole:
    case Qt::DisplayRole:
        return item.name;
    case GenericNameRole:
        return item.category + QStringLiteral(" • ") + item.keywords.join(QLatin1String(", "));
    case CommentRole:
        return item.keywords.join(QLatin1Char(' '));
    case ExecRole:
        return QStringLiteral("__copy__:") + item.emoji;
    case IconNameRole:
        return item.emoji;
    case CategoriesRole:
        return QStringList{item.category};
    case KeywordsRole:
        return item.keywords;
    case DesktopFileRole:
        return QStringLiteral("emoji:") + item.emoji;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> EmojiManager::roleNames() const
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
    return roles;
}

void EmojiManager::loadEmojiDatabase()
{
    const QStringList candidates = {
        QStringLiteral(":/assets/emoji.json"),
        Platform::configDir() + QStringLiteral("/emoji.json"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../share/quasar/emoji.json"),
        QStringLiteral("/usr/share/quasar/emoji.json"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../assets/emoji.json"),
        QStringLiteral("assets/emoji.json")
    };

    QByteArray rawData;
    for (const QString &path : candidates) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            rawData = file.readAll();
            break;
        }
    }

    if (rawData.isEmpty()) {
        qCWarning(lcLauncher) << "Failed to load emoji.json database";
        return;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(rawData, &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
        qCWarning(lcLauncher) << "Failed to parse emoji.json:" << err.errorString();
        return;
    }

    QJsonArray arr = doc.array();
    m_items.reserve(arr.size());

    beginResetModel();
    m_items.clear();
    for (const QJsonValue &val : arr) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();

        EmojiItem item;
        item.emoji = obj.value(QLatin1String("emoji")).toString();
        item.name = obj.value(QLatin1String("name")).toString();
        item.category = obj.value(QLatin1String("category")).toString();
        
        QJsonArray kwArr = obj.value(QLatin1String("keywords")).toArray();
        for (const QJsonValue &kw : kwArr) {
            item.keywords.append(kw.toString());
        }

        if (!item.emoji.isEmpty() && !item.name.isEmpty()) {
            m_items.append(std::move(item));
        }
    }
    endResetModel();

    qCDebug(lcLauncher) << "Loaded" << m_items.size() << "emoji items";
}
