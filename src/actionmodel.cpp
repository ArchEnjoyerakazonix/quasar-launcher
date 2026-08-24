#include "actionmodel.h"
#include "platform.h"
#include "logging.h"

#include <QProcess>
#include <QFile>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>

namespace {

bool isInternalCommand(const QString &command)
{
    return command == QLatin1String("quasar-theme-selector")
        || command == QLatin1String("quasar theme")
        || command == QLatin1String("quasar --theme");
}

// "@scripts/calc.sh" → absolute path of the bundled script, or the original
// token when nothing is found (the command then simply fails at run time).
QString resolveCommandScripts(const QString &command)
{
    const QString prefix = QStringLiteral("@scripts/");
    if (!command.startsWith(prefix))
        return command;
    const QString resolved = ActionModel::resolveScriptPath(command.mid(prefix.size()));
    return resolved.isEmpty() ? command : resolved;
}

} // namespace

QString ActionModel::resolveScriptPath(const QString &scriptName)
{
    const QStringList candidates = {
        Platform::configDir() + QStringLiteral("/scripts/") + scriptName,
        QDir::homePath() + QStringLiteral("/.local/share/quasar/scripts/") + scriptName,
        QCoreApplication::applicationDirPath() + QStringLiteral("/../share/quasar/scripts/") + scriptName,
        QStringLiteral("/usr/local/share/quasar/scripts/") + scriptName,
        QStringLiteral("/usr/share/quasar/scripts/") + scriptName,
        QCoreApplication::applicationDirPath() + QStringLiteral("/../assets/scripts/") + scriptName,
        QStringLiteral("assets/scripts/") + scriptName,
    };
    for (const QString &path : candidates) {
        if (QFile::exists(path))
            return QDir::cleanPath(path);
    }
    return {};
}

ActionModel::ActionModel(QObject *parent)
    : QAbstractListModel(parent)
{
    // Pipe children get the same scrubbed environment as detached launches.
    QProcessEnvironment pipeEnv = QProcessEnvironment::systemEnvironment();
    pipeEnv.remove(QStringLiteral("QT_WAYLAND_SHELL_INTEGRATION"));
    m_pipeProcess.setProcessEnvironment(pipeEnv);

    // When the pipe subprocess finishes: non-empty stdout replaces the menu
    // (first round or round-trip), empty stdout means the script performed
    // the action itself — tell QML to close.
    connect(&m_pipeProcess, &QProcess::finished, this,
        [this](int exitCode, QProcess::ExitStatus status) {
            Q_UNUSED(exitCode);
            const bool selectionRound = m_pipeSelectionRound;
            m_pipeSelectionRound = false;

            if (status != QProcess::NormalExit) {
                qCWarning(lcLauncher) << "pipe process crashed";
                m_pipeActive = false;
                m_pipeModel.clear();
                emit pipeStateChanged();
                emit pipeActionDone();
                return;
            }

            const QStringList lines = QString::fromUtf8(m_pipeProcess.readAllStandardOutput())
                .split(QLatin1Char('\n'), Qt::SkipEmptyParts);

            if (lines.isEmpty()) {
                m_pipeActive = false;
                m_pipeModel.clear();
                emit pipeStateChanged();
                emit pipeActionDone();
                return;
            }

            m_pipeModel.setLines(m_activePipeAction.name, m_activePipeAction.iconName,
                                 m_activePipeAction.description, lines);
            qCDebug(lcLauncher) << "pipe returned" << lines.size() << "items";
            if (selectionRound)
                emit pipeMenuUpdated();
        });

    connect(&m_pipeProcess, &QProcess::errorOccurred, this,
        [this](QProcess::ProcessError error) {
            qCWarning(lcLauncher) << "pipe process error:" << error;
            m_pipeActive = false;
            m_pipeSelectionRound = false;
            m_pipeModel.clear();
            emit pipeStateChanged();
            emit pipeActionDone();
        });

    loadFromConfig();
}

// ---------------------------------------------------------------------------
// Config loading (actions.json)
// ---------------------------------------------------------------------------

QVector<ActionItem> ActionModel::loadActionsJson(const QString &path, bool *ok) const
{
    *ok = false;
    QVector<ActionItem> actions;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return actions;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        qCWarning(lcLauncher) << "invalid actions config (expected JSON array):" << path;
        return actions;
    }

    for (const QJsonValue &val : doc.array()) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();
        ActionItem item;
        item.name = obj["name"].toString();
        item.command = resolveCommandScripts(obj["command"].toString());
        item.iconName = obj["icon"].toString(QStringLiteral("application-x-executable"));
        item.description = obj["description"].toString();
        item.type = obj["type"].toString() == QLatin1String("pipe") ? ActionItem::Pipe
                                                                    : ActionItem::Command;
        item.input = obj["input"].toString();
        if (item.name.startsWith(QLatin1Char('/')) && !item.command.isEmpty()) {
            actions.append(std::move(item));
        }
    }

    *ok = true;
    return actions;
}

void ActionModel::loadFromConfig()
{
    const QString configPath = Platform::configDir() + QStringLiteral("/actions.json");

    bool loaded = false;
    QVector<ActionItem> candidates = loadActionsJson(configPath, &loaded);
    if (loaded) {
        qCDebug(lcLauncher) << "loaded" << candidates.size() << "actions from" << configPath;
    } else {
        loadDefaults();
        return;
    }

    for (const ActionItem &item : std::as_const(candidates)) {
        // Pipe actions are always visible — they are invoked dynamically.
        if (item.type == ActionItem::Pipe) {
            m_actions.append(item);
            continue;
        }
        if (isInternalCommand(item.command) || Platform::commandAvailable(item.command)) {
            m_actions.append(item);
        } else {
            qCDebug(lcLauncher) << "hiding action" << item.name
                                << "- command not available:" << item.command;
        }
    }
}

void ActionModel::loadDefaults()
{
    // No hardcoded default actions — actions are cleanly user-defined via ~/.config/quasar/actions.json
}

// ---------------------------------------------------------------------------
// QAbstractListModel interface
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Command execution
// ---------------------------------------------------------------------------

void ActionModel::execute(const QString &command)
{
    if (command.isEmpty()) return;
    Platform::detachedStart("bash", {"-c", command});
}

// ---------------------------------------------------------------------------
// Pipe plugin support (dmenu / rofi script mode compatibility)
// ---------------------------------------------------------------------------

bool ActionModel::isPipeAction(const QString &actionName) const
{
    for (const ActionItem &item : std::as_const(m_actions)) {
        if (item.name == actionName && item.type == ActionItem::Pipe) {
            return true;
        }
    }
    return false;
}

QStringList ActionModel::pipeActionNames() const
{
    QStringList names;
    for (const ActionItem &item : std::as_const(m_actions)) {
        if (item.type == ActionItem::Pipe) {
            names.append(item.name);
        }
    }
    return names;
}

// Runs: bash -c '<command>' quasar [argument]
// The script sees the argument (menu line on round-trip, query remainder on
// "input": "query" first calls) as $1.
void ActionModel::startPipeProcess(const QString &argument)
{
    if (m_pipeProcess.state() != QProcess::NotRunning) {
        m_pipeProcess.kill();
        m_pipeProcess.waitForFinished(500);
    }

    if (!argument.isEmpty()) {
        m_pipeProcess.start("bash", {"-c", m_activePipeAction.command,
                                     QStringLiteral("quasar"), argument});
    } else {
        m_pipeProcess.start("bash", {"-c", m_activePipeAction.command,
                                     QStringLiteral("quasar")});
    }
}

void ActionModel::triggerPipeAction(const QString &actionName, const QString &remainder)
{
    const ActionItem *pipeAction = nullptr;
    for (const ActionItem &item : std::as_const(m_actions)) {
        if (item.name == actionName && item.type == ActionItem::Pipe) {
            pipeAction = &item;
            break;
        }
    }
    if (!pipeAction) {
        qCWarning(lcLauncher) << "triggerPipeAction: no pipe action named" << actionName;
        return;
    }

    m_pipeActive = true;
    m_pipeSelectionRound = false;
    m_pipeFilterByRemainder = pipeAction->input != QLatin1String("query");
    m_activePipeAction = *pipeAction;

    qCDebug(lcLauncher) << "pipe: running" << pipeAction->command
                        << (pipeAction->input == QLatin1String("query") ? "(input: query)" : "");
    m_pipeModel.clear();
    emit pipeStateChanged();

    startPipeProcess(pipeAction->input == QLatin1String("query") ? remainder : QString());
}

void ActionModel::selectPipeItem(const QString &line)
{
    if (!m_pipeActive) {
        qCWarning(lcLauncher) << "selectPipeItem: no active pipe action";
        return;
    }

    m_pipeSelectionRound = true;
    qCDebug(lcLauncher) << "pipe: round-trip with" << line;
    startPipeProcess(line);
}
