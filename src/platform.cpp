#include "platform.h"

#include <QStandardPaths>
#include <QProcess>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QGuiApplication>
#include <QScreen>
#include <QCursor>

namespace Platform {

bool haveBinary(const QString &name)
{
    return !QStandardPaths::findExecutable(name).isEmpty();
}

Compositor detectCompositor()
{
    static const Compositor detected = []() -> Compositor {
        if (qEnvironmentVariableIsSet("HYPRLAND_INSTANCE_SIGNATURE") && haveBinary(QStringLiteral("hyprctl")))
            return Compositor::Hyprland;
        if (qEnvironmentVariableIsSet("SWAYSOCK") && haveBinary(QStringLiteral("swaymsg")))
            return Compositor::Sway;
        // Hyprland/Sway without their env vars set (e.g. nested) still work if
        // the IPC binary is present.
        if (haveBinary(QStringLiteral("hyprctl")) && qEnvironmentVariableIsSet("WAYLAND_DISPLAY"))
            return Compositor::Hyprland;
        if (haveBinary(QStringLiteral("swaymsg")) && qEnvironmentVariableIsSet("WAYLAND_DISPLAY"))
            return Compositor::Sway;
        if (!qEnvironmentVariableIsSet("WAYLAND_DISPLAY"))
            return Compositor::X11;
        return Compositor::Unknown;
    }();
    return detected;
}

QString compositorName(Compositor c)
{
    switch (c) {
    case Compositor::Hyprland: return QStringLiteral("Hyprland");
    case Compositor::Sway:     return QStringLiteral("Sway");
    case Compositor::X11:      return QStringLiteral("X11");
    case Compositor::Unknown:  return QStringLiteral("Unknown");
    }
    return QStringLiteral("Unknown");
}

QString commandProgram(const QString &commandLine)
{
    const QString trimmed = commandLine.trimmed();
    if (trimmed.isEmpty())
        return {};
    const QStringList parts = QProcess::splitCommand(trimmed);
    return parts.isEmpty() ? QString() : parts.first();
}

bool commandAvailable(const QString &commandLine)
{
    const QString program = commandProgram(commandLine);
    if (program.isEmpty())
        return false;
    // Absolute paths must point to an existing executable file.
    if (program.startsWith(QLatin1Char('/')))
        return QFileInfo::exists(program);
    return haveBinary(program);
}

// POSIX single-quote shell quoting: 'it'"'"'s' style, safe for bash -c.
static QString shellQuote(const QString &arg)
{
    QString quoted(arg);
    quoted.replace(QLatin1Char('\''), QStringLiteral("'\\''"));
    return QLatin1Char('\'') + quoted + QLatin1Char('\'');
}

QString terminalCommand(const QString &shellCmd)
{
    static const QString terminal = []() -> QString {
        const QStringList candidates = {
            qEnvironmentVariable("TERMINAL"), // user preference first
            QStringLiteral("xdg-terminal-exec"), // freedesktop standard
            QStringLiteral("ghostty"),
            QStringLiteral("kitty"),
            QStringLiteral("alacritty"),
            QStringLiteral("foot"),
            QStringLiteral("wezterm"),
            QStringLiteral("konsole"),
            QStringLiteral("gnome-terminal"),
            QStringLiteral("ptyxis"),
            QStringLiteral("kgx"),
            QStringLiteral("xfce4-terminal"),
            QStringLiteral("urxvt"),
            QStringLiteral("xterm"),
        };
        for (const QString &t : candidates) {
            if (!t.isEmpty() && !t.contains(QLatin1Char('/')) && haveBinary(t))
                return t;
            if (t.contains(QLatin1Char('/')) && QFileInfo::exists(t))
                return t;
        }
        return {};
    }();

    if (terminal.isEmpty())
        return {};

    const QString quoted = shellQuote(shellCmd);
    if (terminal == QLatin1String("xdg-terminal-exec"))
        return QStringLiteral("%1 bash -c %2").arg(terminal, quoted);
    if (terminal == QLatin1String("gnome-terminal") || terminal == QLatin1String("ptyxis") || terminal == QLatin1String("kgx"))
        return QStringLiteral("%1 -- bash -c %2").arg(terminal, quoted);
    if (terminal == QLatin1String("wezterm"))
        return QStringLiteral("%1 start -- bash -c %2").arg(terminal, quoted);
    return QStringLiteral("%1 -e bash -c %2").arg(terminal, quoted);
}

QString configDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/quasar");
}

bool detachedStart(const QString &program, const QStringList &args)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove(QStringLiteral("QT_WAYLAND_SHELL_INTEGRATION"));
    QProcess process;
    process.setProgram(program);
    process.setArguments(args);
    process.setProcessEnvironment(env);
    return process.startDetached();
}

QScreen *focusedScreen()
{
    if (detectCompositor() == Compositor::Hyprland) {
        QProcess proc;
        proc.start(QStringLiteral("hyprctl"), {QStringLiteral("monitors"), QStringLiteral("-j")});
        if (proc.waitForFinished(100)) {
            QByteArray out = proc.readAllStandardOutput();
            QJsonDocument doc = QJsonDocument::fromJson(out);
            if (doc.isArray()) {
                for (const QJsonValue &val : doc.array()) {
                    QJsonObject obj = val.toObject();
                    if (obj.value(QStringLiteral("focused")).toBool()) {
                        QString focusedName = obj.value(QStringLiteral("name")).toString();
                        for (QScreen *s : QGuiApplication::screens()) {
                            if (s && s->name() == focusedName) {
                                return s;
                            }
                        }
                    }
                }
            }
        }
    }

    QScreen *cursorScreen = QGuiApplication::screenAt(QCursor::pos());
    if (cursorScreen) return cursorScreen;
    return QGuiApplication::primaryScreen();
}

} // namespace Platform
