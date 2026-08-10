#include "actionmodel.h"
#include <QProcess>
#include <QDebug>

ActionModel::ActionModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_actions = {
        {"/accentcolor", "quasar theme", "color-management", "Open Quasar Theme Selector & Customizer"},
        {"/dark", "bash /home/archuser/.config/quickshell/ii/scripts/colors/switchwall.sh --mode dark --noswitch", "weather-clear-night", "Switch system color scheme to Dark mode"},
        {"/light", "bash /home/archuser/.config/quickshell/ii/scripts/colors/switchwall.sh --mode light --noswitch", "weather-clear", "Switch system color scheme to Light mode"},
        {"/wallpaper", "bash /home/archuser/.config/hypr/scripts/wallpaper-picker.sh", "preferences-desktop-wallpaper", "Open QuickSwitcher Wallpaper Hub"},
        {"/todo", "hyprctl dispatch togglespecialworkspace todo", "task-accepted", "Toggle Todo Special Workspace"},
        {"/wipeclipboard", "cliphist wipe", "edit-clear", "Wipe clipboard history"},
        {"/terminal", "kitty", "utilities-terminal", "Launch Kitty Terminal"}
    };
}

int ActionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_actions.size();
}

QVariant ActionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_actions.size())
        return QVariant();

    const auto &action = m_actions.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case NameRole:
        return action.name;
    case ExecRole:
        return action.command;
    case IconNameRole:
        return action.iconName;
    case CommentRole:
        return action.description;
    case CategoryRole:
        return QString("Action");
    case DesktopFileRole:
        return action.name;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ActionModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {ExecRole, "exec"},
        {IconNameRole, "iconName"},
        {CommentRole, "comment"},
        {CategoryRole, "categories"},
        {DesktopFileRole, "desktopFile"}
    };
}

void ActionModel::execute(const QString &command)
{
    if (command.isEmpty()) return;
    QProcess::startDetached("bash", QStringList() << "-c" << command);
}
