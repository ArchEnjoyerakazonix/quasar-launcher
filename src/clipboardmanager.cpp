#include "clipboardmanager.h"
#include "platform.h"
#include "logging.h"

#include <QGuiApplication>
#include <QProcess>
#include <QRegularExpression>
#include <QColor>

ClipboardManager::ClipboardManager(QObject *parent)
    : QAbstractListModel(parent)
{
    if (QClipboard *clipboard = QGuiApplication::clipboard()) {
        connect(clipboard, &QClipboard::dataChanged, this, &ClipboardManager::onSystemClipboardChanged);
    }
}

int ClipboardManager::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_items.size();
}

QVariant ClipboardManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return QVariant();

    const ClipboardItem &item = m_items.at(index.row());
    switch (role) {
    case NameRole:
    case Qt::DisplayRole:
        return item.preview;
    case GenericNameRole:
        if (item.isColor) {
            return QStringLiteral("Color • %1 • Press Enter to copy").arg(item.colorHex);
        }
        return QStringLiteral("Clipboard • Press Enter to paste/copy");
    case CommentRole:
        return item.fullText;
    case ExecRole:
        return QStringLiteral("__clipboard__:%1").arg(index.row());
    case IconNameRole:
        return QStringLiteral("edit-copy");
    case CategoriesRole:
        return QStringList{QStringLiteral("Clipboard")};
    case KeywordsRole:
        return QStringList{QStringLiteral("clipboard"), QStringLiteral("copy"), QStringLiteral("paste")};
    case DesktopFileRole:
        return QStringLiteral("clipboard:%1").arg(item.id.isEmpty() ? QString::number(index.row()) : item.id);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ClipboardManager::roleNames() const
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

void ClipboardManager::onSystemClipboardChanged()
{
    if (QClipboard *clipboard = QGuiApplication::clipboard()) {
        QString text = clipboard->text();
        if (text.trimmed().isEmpty()) return;

        // Check if already top item
        if (!m_items.isEmpty() && m_items.first().fullText == text) {
            return;
        }

        static const QRegularExpression hexRegex(QStringLiteral(R"(^#([0-9a-fA-F]{3}|[0-9a-fA-F]{6}|[0-9a-fA-F]{8})$)"));

        ClipboardItem item;
        item.id = QString::number(m_items.size() + 1);
        item.fullText = text;
        item.preview = text.trimmed().split(QLatin1Char('\n')).first();
        if (item.preview.length() > 80) {
            item.preview = item.preview.left(77) + QStringLiteral("...");
        }

        if (hexRegex.match(text.trimmed()).hasMatch()) {
            item.isColor = true;
            item.colorHex = text.trimmed();
        }

        beginInsertRows(QModelIndex(), 0, 0);
        m_items.prepend(std::move(item));
        if (m_items.size() > 50) {
            m_items.removeLast();
        }
        endInsertRows();
    }
}

void ClipboardManager::refresh()
{
    if (Platform::haveBinary(QStringLiteral("cliphist"))) {
        fetchFromCliphist();
    } else {
        onSystemClipboardChanged();
    }
}

void ClipboardManager::fetchFromCliphist()
{
    QProcess proc;
    proc.start(QStringLiteral("cliphist"), {QStringLiteral("list")});
    if (!proc.waitForFinished(1000)) {
        proc.kill();
        return;
    }

    const QString output = QString::fromUtf8(proc.readAllStandardOutput());
    const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);

    static const QRegularExpression hexRegex(QStringLiteral(R"(^#([0-9a-fA-F]{3}|[0-9a-fA-F]{6}|[0-9a-fA-F]{8})$)"));

    QVector<ClipboardItem> newItems;
    newItems.reserve(std::min(lines.size(), qsizetype(50)));

    for (qsizetype i = 0; i < lines.size() && i < 50; ++i) {
        const QString &line = lines.at(i);
        int tabIdx = line.indexOf(QLatin1Char('\t'));
        if (tabIdx == -1) continue;

        QString id = line.left(tabIdx).trimmed();
        QString preview = line.mid(tabIdx + 1).trimmed();

        ClipboardItem item;
        item.id = id;
        item.preview = preview.isEmpty() ? QStringLiteral("[Binary / Image / Empty]") : preview;
        if (item.preview.length() > 80) {
            item.preview = item.preview.left(77) + QStringLiteral("...");
        }

        if (hexRegex.match(preview).hasMatch()) {
            item.isColor = true;
            item.colorHex = preview;
        }

        newItems.append(std::move(item));
    }

    beginResetModel();
    m_items = std::move(newItems);
    endResetModel();
}

void ClipboardManager::restoreItem(int index)
{
    if (index < 0 || index >= m_items.size()) return;

    const ClipboardItem &item = m_items.at(index);
    if (!item.id.isEmpty() && Platform::haveBinary(QStringLiteral("cliphist"))) {
        // Decode and pipe to wl-copy
        QString cmd = QStringLiteral("cliphist decode %1 | wl-copy").arg(item.id);
        Platform::detachedStart(QStringLiteral("bash"), {QStringLiteral("-c"), cmd});
    } else if (!item.fullText.isEmpty()) {
        if (QClipboard *clipboard = QGuiApplication::clipboard()) {
            clipboard->setText(item.fullText);
        }
    }
}
