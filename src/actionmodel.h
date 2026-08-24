#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QProcess>

struct ActionItem {
    QString name;
    QString command;
    QString iconName;
    QString description;
    // Command — executed directly on selection (default)
    // Pipe — dmenu/rofi script mode: first call (no arguments) prints the
    //        menu lines; selecting a line re-invokes the same command with
    //        the line as $1 — it may either print a follow-up menu (multi-step)
    //        or perform the action and print nothing (launcher closes).
    enum ActionType { Command, Pipe } type = Command;
    // "query" — pass the query remainder after the action name as $1 to the
    // first invocation too (e.g. "/calc 2+2" → calc.sh "2+2"); list filtering
    // by the remainder is disabled for such actions.
    QString input;
};

class ActionModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(bool pipeActive READ isPipeActive NOTIFY pipeStateChanged)
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ExecRole,
        IconNameRole,
        CommentRole,
        CategoryRole,
        DesktopFileRole
    };

    explicit ActionModel(QObject *parent = nullptr);
    ~ActionModel() override = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void execute(const QString &command);

    // Pipe plugin support (dmenu/rofi script mode compatibility).
    Q_INVOKABLE bool isPipeAction(const QString &actionName) const;
    Q_INVOKABLE QStringList pipeActionNames() const;
    // Starts the script. `remainder` is passed as $1 for "input": "query"
    // actions (e.g. the calculator expression).
    Q_INVOKABLE void triggerPipeAction(const QString &actionName, const QString &remainder = QString());
    // Round-trip: re-invokes the active pipe script with `line` as $1.
    // A non-empty stdout replaces the menu (multi-step); empty stdout means
    // the script performed the action — pipeActionDone() is emitted.
    Q_INVOKABLE void selectPipeItem(const QString &line);
    // False when the active action uses "input": "query" — the script already
    // consumed the remainder, so list filtering must be disabled.
    Q_INVOKABLE bool pipeFiltersByRemainder() const { return m_pipeFilterByRemainder; }

    bool isPipeActive() const { return m_pipeActive; }
    QAbstractListModel *pipeResultModel() { return &m_pipeModel; }

    // Resolves the "@scripts/<name>" prefix in commands to a real path:
    // ~/.config/quasar/scripts/, then the installed share/quasar/scripts/,
    // then the build-tree assets/scripts/.
    static QString resolveScriptPath(const QString &scriptName);

signals:
    void pipeStateChanged();
    // A round-trip call produced a new menu; the current list is replaced.
    void pipeMenuUpdated();
    // The script consumed the selection and printed nothing — close.
    void pipeActionDone();

private:
    void loadFromConfig();
    void loadDefaults();
    QVector<ActionItem> loadActionsJson(const QString &path, bool *ok) const;
    void startPipeProcess(const QString &argument);

    QVector<ActionItem> m_actions;

    bool m_pipeActive = false;
    bool m_pipeSelectionRound = false;
    bool m_pipeFilterByRemainder = true;
    ActionItem m_activePipeAction;
    QProcess m_pipeProcess;

    // Result rows for the active pipe action: one row per stdout line.
    // ExecRole is "__pipe__:<line>" so QML can route selection through
    // selectPipeItem() instead of executing the line directly.
    class PipeResultModel : public QAbstractListModel {
    public:
        enum Roles {
            NameRole = Qt::UserRole + 1,
            ExecRole,
            IconNameRole,
            CommentRole,
            CategoryRole,
            DesktopFileRole
        };
        explicit PipeResultModel(QObject *parent = nullptr)
            : QAbstractListModel(parent) {}

        int rowCount(const QModelIndex &parent = QModelIndex()) const override {
            return parent.isValid() ? 0 : m_lines.size();
        }
        QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override {
            if (!idx.isValid() || idx.row() < 0 || idx.row() >= m_lines.size())
                return QVariant();
            const QString &line = m_lines.at(idx.row());
            switch (role) {
            case NameRole:
            case Qt::DisplayRole:
                return line;
            case ExecRole:
                return QStringLiteral("__pipe__:") + line;
            case DesktopFileRole:
                return QStringLiteral("pipe:%1:%2").arg(m_actionName, line);
            case CommentRole:
                return m_comment;
            case IconNameRole:
                return m_iconName;
            case CategoryRole:
                return QStringLiteral("Action");
            }
            return QVariant();
        }
        QHash<int, QByteArray> roleNames() const override {
            return {
                {NameRole, "name"},
                {ExecRole, "exec"},
                {IconNameRole, "iconName"},
                {CommentRole, "comment"},
                {CategoryRole, "categories"},
                {DesktopFileRole, "desktopFile"}
            };
        }

        void setLines(const QString &actionName, const QString &iconName,
                      const QString &comment, const QStringList &lines) {
            beginResetModel();
            m_actionName = actionName;
            m_iconName = iconName;
            m_comment = comment;
            m_lines = lines;
            endResetModel();
        }
        void clear() {
            beginResetModel();
            m_lines.clear();
            endResetModel();
        }

    private:
        QString m_actionName;
        QString m_iconName;
        QString m_comment;
        QStringList m_lines;
    };

    PipeResultModel m_pipeModel;
};
