#include "windowswitcher.h"
#include "platform.h"
#include "logging.h"

#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

#include <functional>

WindowListModel::WindowListModel(QObject *parent)
    : QAbstractListModel(parent)
{
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
    case GenericNameRole:
        return QString("%1 | Workspace %2").arg(item.cls, item.workspace);
    case CommentRole:
        return item.cls;
    case ExecRole:
        return QString("address:%1|workspace:%2").arg(item.address, item.workspace);
    case IconNameRole:
        return item.cls.toLower();
    case CategoriesRole:
        return QStringList{QStringLiteral("Windows")};
    case KeywordsRole:
        return QStringList{item.cls, item.title, QStringLiteral("window")};
    case DesktopFileRole:
        return QString("window:%1").arg(item.address);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> WindowListModel::roleNames() const
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

void WindowListModel::refresh()
{
    if (m_refreshPending) {
        return;
    }

    using Backend = Platform::Compositor;
    const Backend backend = Platform::detectCompositor();

    auto startQuery = [this](const QString &program, const QStringList &args,
                             const std::function<QList<WindowItem>(const QByteArray &)> &parser) {
        auto *process = new QProcess(this);
        connect(process, &QProcess::finished, this,
                [this, process, parser](int exitCode, QProcess::ExitStatus status) {
                    Q_UNUSED(exitCode);
                    Q_UNUSED(status);
                    process->deleteLater();
                    m_refreshPending = false;
                    applyWindows(parser(process->readAllStandardOutput()));
                });
        connect(process, &QProcess::errorOccurred, this,
                [this, process](QProcess::ProcessError error) {
                    qCWarning(lcWindows) << "window query process failed:" << error;
                    process->deleteLater();
                    m_refreshPending = false;
                    applyWindows({});
                });
        m_refreshPending = true;
        process->start(program, args);
    };

    switch (backend) {
    case Backend::Hyprland:
        startQuery(QStringLiteral("hyprctl"), {"clients", "-j"}, &WindowListModel::parseHyprlandClients);
        break;
    case Backend::Sway:
        startQuery(QStringLiteral("swaymsg"), {"-t", "get_tree", "-r"}, &WindowListModel::parseSwayTree);
        break;
    case Backend::X11:
        if (Platform::haveBinary(QStringLiteral("wmctrl"))) {
            startQuery(QStringLiteral("wmctrl"), {"-l"}, &WindowListModel::parseWmctrlList);
        } else {
            qCDebug(lcWindows) << "wmctrl not found: window switching disabled on X11";
            applyWindows({});
        }
        break;
    case Backend::Unknown:
        qCDebug(lcWindows) << "unsupported compositor: window list disabled";
        applyWindows({});
        break;
    }
}

void WindowListModel::applyWindows(QList<WindowItem> windows)
{
    beginResetModel();
    m_windows = std::move(windows);
    endResetModel();
}

QList<WindowItem> WindowListModel::parseHyprlandClients(const QByteArray &output)
{
    QList<WindowItem> windows;
    QJsonDocument doc = QJsonDocument::fromJson(output);
    if (!doc.isArray()) {
        return windows;
    }
    const QJsonArray array = doc.array();
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
        windows.append(std::move(win));
    }
    return windows;
}

QList<WindowItem> WindowListModel::parseSwayTree(const QByteArray &output)
{
    QList<WindowItem> windows;
    QJsonDocument doc = QJsonDocument::fromJson(output);
    if (!doc.isObject()) {
        return windows;
    }

    // Depth-first walk over containers; sway addresses are numeric node ids.
    std::function<void(const QJsonObject &, const QString &)> walk =
        [&](const QJsonObject &node, const QString &workspace) {
            QString currentWorkspace = node["name"].toString();
            const QString type = node["type"].toString();

            if (type == QLatin1String("con") || type == QLatin1String("floating_con")) {
                const QString appPidClass = node["app_id"].toString();
                QJsonObject windowProps = node["window_properties"].toObject();
                QString cls = !appPidClass.isEmpty() ? appPidClass
                                                     : windowProps["class"].toString();
                QString title = node["name"].toString();

                if (!title.isEmpty() || !cls.isEmpty()) {
                    WindowItem win;
                    win.title = title;
                    win.cls = cls.isEmpty() ? title : cls;
                    win.address = QString::number(node["id"].toInteger());
                    win.workspace = workspace;
                    windows.append(std::move(win));
                }
                // "name" of a regular container is its title, not a workspace
                currentWorkspace = workspace;
            }

            for (const QJsonValue &child : node["nodes"].toArray())
                if (child.isObject()) walk(child.toObject(), currentWorkspace);
            for (const QJsonValue &child : node["floating_nodes"].toArray())
                if (child.isObject()) walk(child.toObject(), currentWorkspace);
        };

    walk(doc.object(), QString());
    return windows;
}

QList<WindowItem> WindowListModel::parseWmctrlList(const QByteArray &output)
{
    // Format: <window-id> <desktop> <host> <client machine> <window title...>
    QList<WindowItem> windows;
    const QString text = QString::fromUtf8(output);
    for (const QString &line : text.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (parts.size() < 4 || !parts.first().startsWith(QLatin1String("0x"))) {
            continue;
        }
        WindowItem win;
        win.address = parts.first();
        win.workspace = parts.value(1);
        win.title = QStringList(parts.mid(3)).join(QLatin1Char(' '));
        win.cls = win.title;
        if (!win.title.isEmpty()) {
            windows.append(std::move(win));
        }
    }
    return windows;
}

WindowSwitcher::WindowSwitcher(QObject *parent)
    : QObject(parent), m_model(this)
{
    // Populate asynchronously once the event loop is running.
    QMetaObject::invokeMethod(&m_model, "refresh", Qt::QueuedConnection);
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
    Q_UNUSED(query);
    return getOpenWindows();
}

bool WindowSwitcher::focusWindow(const QString &address)
{
    if (address.isEmpty()) return false;
    QString clean = address;
    if (clean.startsWith("address:")) {
        clean = clean.mid(8).trimmed();
    }
    QString addr = clean;
    QString ws;
    int sep = clean.indexOf('|');
    if (sep != -1) {
        addr = clean.left(sep).trimmed();
        QString extra = clean.mid(sep + 1).trimmed();
        if (extra.startsWith("workspace:")) {
            ws = extra.mid(10).trimmed();
        }
    }

    using Backend = Platform::Compositor;
    switch (Platform::detectCompositor()) {
    case Backend::Hyprland: {
        QString luaCmd;
        if (!ws.isEmpty()) {
            luaCmd = QString("hl.dispatch(hl.dsp.focus({ workspace = '%1' })); hl.dispatch(hl.dsp.focus({ window = 'address:%2' }))").arg(ws, addr);
        } else {
            luaCmd = QString("hl.dispatch(hl.dsp.focus({ window = 'address:%1' }))").arg(addr);
        }
        QProcess::startDetached("hyprctl", {"eval", luaCmd});
        return true;
    }
    case Backend::Sway:
        QProcess::startDetached("swaymsg", {"[con_id=" + addr + "] focus"});
        return true;
    case Backend::X11:
        if (Platform::haveBinary(QStringLiteral("wmctrl"))) {
            QProcess::startDetached("wmctrl", {"-i", "-a", addr});
            return true;
        }
        break;
    case Backend::Unknown:
        break;
    }
    qCWarning(lcWindows) << "focusWindow: no supported compositor backend";
    return false;
}
