#include "windowswitcher.h"
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

WindowListModel::WindowListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    refresh();
}

int WindowListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_windows.size();
}

QVariant WindowListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_windows.size()) {
        return QVariant();
    }

    const WindowItem &item = m_windows.at(index.row());
    switch (role) {
    case NameRole:
    case Qt::DisplayRole:
        return item.title.isEmpty() ? item.cls : item.title;
    case ExecRole:
        return QString("address:%1").arg(item.address);
    case DesktopFileRole:
        return QString("window:%1").arg(item.address);
    case GenericNameRole:
        return QString("%1 | Workspace %2").arg(item.cls, item.workspace);
    case IconNameRole:
        return item.cls.toLower();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> WindowListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[ExecRole] = "exec";
    roles[DesktopFileRole] = "desktopFile";
    roles[GenericNameRole] = "genericName";
    roles[IconNameRole] = "iconName";
    return roles;
}

void WindowListModel::refresh()
{
    beginResetModel();
    m_windows.clear();

    QProcess process;
    process.start("hyprctl", QStringList() << "clients" << "-j");
    if (process.waitForFinished(1000)) {
        QByteArray output = process.readAllStandardOutput();
        QJsonDocument doc = QJsonDocument::fromJson(output);
        if (doc.isArray()) {
            QJsonArray array = doc.array();
            for (const QJsonValue &val : array) {
                if (!val.isObject()) continue;
                QJsonObject obj = val.toObject();

                QString title = obj["title"].toString();
                QString cls = obj["class"].toString();
                QString address = obj["address"].toString();
                bool mapped = obj["mapped"].toBool(true);

                if (!mapped || (title.isEmpty() && cls.isEmpty())) {
                    continue;
                }

                WindowItem win;
                win.title = title;
                win.cls = cls;
                win.address = address;
                win.workspace = obj["workspace"].toObject()["name"].toString();
                m_windows.append(win);
            }
        }
    }

    endResetModel();
}

WindowSwitcher::WindowSwitcher(QObject *parent)
    : QObject(parent), m_model(this)
{
}

QVariantList WindowSwitcher::getOpenWindows()
{
    m_model.refresh();
    QVariantList windows;
    for (int i = 0; i < m_model.rowCount(); ++i) {
        QModelIndex idx = m_model.index(i, 0);
        QVariantMap win;
        win["title"] = m_model.data(idx, WindowListModel::NameRole).toString();
        win["class"] = m_model.data(idx, WindowListModel::GenericNameRole).toString();
        win["address"] = m_model.data(idx, WindowListModel::ExecRole).toString().mid(8);
        win["iconName"] = m_model.data(idx, WindowListModel::IconNameRole).toString();
        windows.append(win);
    }
    return windows;
}

QVariantList WindowSwitcher::getMatchingWindows(const QString &query)
{
    return getOpenWindows();
}

bool WindowSwitcher::focusWindow(const QString &address)
{
    if (address.isEmpty()) return false;
    QString cleanAddr = address;
    if (cleanAddr.startsWith("address:")) {
        cleanAddr = cleanAddr.mid(8);
    }
    QProcess::startDetached("hyprctl", QStringList() << "dispatch" << "focuswindow" << QString("address:%1").arg(cleanAddr));
    return true;
}
