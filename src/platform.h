#pragma once

#include <QString>
#include <QStringList>

class QScreen;

namespace Platform {

// True if `name` resolves to an executable in $PATH.
bool haveBinary(const QString &name);

enum class Compositor {
    Hyprland, // Wayland, hyprctl IPC
    Sway,     // Wayland, swaymsg IPC
    X11,      // X11: KDE/GNOME/XFCE etc., wmctrl/xdotool compatible
    Unknown
};

Compositor detectCompositor();

// Human-readable compositor name for logging.
QString compositorName(Compositor c);

// First word of a shell command line ("bash /path/script.sh" -> "bash").
QString commandProgram(const QString &commandLine);

// Whether the program a command line invokes exists in $PATH.
// Commands with arguments are supported; empty/quoted-only lines return false.
bool commandAvailable(const QString &commandLine);

// Builds a full command line that runs `shellCmd` inside the first available
// terminal emulator ($TERMINAL, xdg-terminal-exec, konsole, gnome-terminal,
// kgx, kitty, alacritty, wezterm, xterm). Returns an empty string when no
// terminal is installed.
QString terminalCommand(const QString &shellCmd);

// Standard user config directory for quasar (~/.config/quasar).
QString configDir();

// Detached process start with launcher-specific environment scrubbed:
// children (theme selector, user commands, apps) must not inherit
// QT_WAYLAND_SHELL_INTEGRATION=layer-shell, or their windows get mapped as
// full-screen layer surfaces.
bool detachedStart(const QString &program, const QStringList &args);

// Detects the currently focused monitor in Hyprland/Sway/X11
QScreen *focusedScreen();

} // namespace Platform
