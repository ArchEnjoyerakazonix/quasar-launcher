# Quasar Launcher

Sleek rofi-style application launcher built with Qt6/QML. Works on Wayland
(Hyprland, Sway and other layer-shell compositors) and X11 (KDE, GNOME, XFCE…),
and adapts gracefully to whatever tools the distro provides.

## Features

- **Application Search**: Fuzzy matching with Damerau-Levenshtein distance, Cyrillic / QWERTY layout auto-translation, and frecency ranking.
- **Live Inline Math Calculator**: Evaluate expressions in real-time (`125 * 8`, `(50 + 25) / 3`, `sqrt(144)`, `2^8`, `sin(pi/2)`) — press Enter to copy the answer.
- **Emoji & Unicode Picker**: Fast search across 1800+ emojis and symbols with tags (`e.fire`, `emoji:cat`, `:rocket`, `e.tux`).
- **Smart Clipboard History**: Instant clipboard viewer & search with image / color preview (`c.`, `clip.`, `cb.`) powered by `cliphist` / `wl-paste`.
- **Window Switching**: Fast window switcher (`w.` / `w:` / `window:`) supporting Hyprland, Sway, and X11 (`wmctrl`).
- **Slash Actions & Pipe Plugins**: Command execution from `/` (e.g. `/wal`, `/dark`, `/screenshot`) and dmenu-style pipe scripts (`actions.json`).
- **Shell Command Runner**: Run commands (`$cmd`, `>cmd`) in any detected terminal (`ghostty`, `kitty`, `alacritty`, `foot`, `ptyxis`, `konsole`, etc.).
- **Web Search**: Query search with default system browser (`?query`, `g:query`, `web:query`).
- **Dynamic Pywal / Matugen Theme Sync**: Sync Quasar theme with wallpaper colors on the fly (`/wal` or `ThemeManager::syncPywal`).
- **33+ Built-in Glassmorphism Themes**: Live real-time tuning and customization via `quasar-theme-selector`.
- **Single Instance D-Bus Daemon**: Lightning-fast toggle (`quasar --toggle` or `quasar -t`).

## Query Prefixes & Shortcuts

| Prefix | Mode | Example |
|---|---|---|
| *(none)* | App Search & Math | `firefox`, `stema`, `125 * 8`, `sqrt(144)` |
| `e.` / `emoji.` / `:` | Emoji & Symbols Picker | `e.fire`, `emoji:cat`, `:rocket` |
| `c.` / `clip.` / `cb.` | Clipboard History | `c.`, `c.token`, `clip:url` |
| `w.` / `w:` / `window:` | Active Window Switcher | `w.fire`, `w:code`, `window:terminal` |
| `/` | Slash Actions & Pipe Plugins | `/wal`, `/dark`, `/calc`, `/screenshot` |
| `$` / `>` | Shell Command in Terminal | `$btop`, `>ping -c 3 google.com` |
| `?` / `g:` / `web:` | Web Search in Default Browser | `?arch linux wiki`, `web:github` |

## Build

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
```

LayerShellQt is optional: without it the launcher builds and runs in fallback
mode (frameless centered window). With it, the launcher becomes a proper
layer-shell overlay on Wayland.

### Build dependencies

| Distro | Packages |
|---|---|
| Arch / Manjaro | `qt6-base qt6-declarative qt6-svg cmake ninja layer-shell-qt` |
| Ubuntu 24.04+ / Debian trixie+ | `qt6-base-dev qt6-declarative-dev qt6-svg-dev cmake ninja-build liblayershellqtinterface-dev qml6-module-qtqml qml6-module-qtqml-workerscript qml6-module-qtqml-models qml6-module-qtquick qml6-module-qtquick-controls qml6-module-qtquick-layouts qml6-module-qtquick-templates qml6-module-qtquick-window qml6-module-qtquick-shapes` |
| Fedora 40+ | `qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtquickcontrols2-devel qt6-qtsvg-devel cmake ninja-build gcc-c++` |

Note: on distros where `liblayershellqtinterface-dev` / `layer-shell-qt-devel`
is unavailable, simply omit it — CMake will warn and continue without layer-shell.

### Install

```sh
sudo cmake --install build
```

Or on Arch/Manjaro: `makepkg -si` from `packaging/`.

## Runtime dependencies

The launcher degrades gracefully when a tool is missing — the related feature
is hidden or disabled, nothing crashes:

| Tool | Feature | Fallback when missing |
|---|---|---|
| `hyprctl` | `w:` window list on Hyprland | next backend (swaymsg / wmctrl), feature disabled |
| `swaymsg` | `w:` window list on Sway | next backend, feature disabled |
| `wmctrl` | `w:` window list on X11 | feature disabled |
| terminal emulator | `$command` runner | `$TERMINAL`, `xdg-terminal-exec`, konsole, gnome-terminal, kgx, kitty, alacritty, wezterm, foot, xterm |
| `gsettings` | `/dark`, `/light` default actions | actions hidden |
| `cliphist` | `/wipeclipboard` default action | action hidden |

## Configuration

- `~/.config/quasar/theme.json` — current theme (written by the theme selector)
- `~/.config/quasar/actions.json` — custom slash actions. When present it fully
  replaces the built-in defaults. See `assets/actions.json.example`:

```json
[
    {
        "name": "/lock",
        "command": "loginctl lock-session",
        "icon": "system-lock-screen",
        "description": "Lock the current session"
    }
]
```

Actions whose command binary is not found in `$PATH` are hidden automatically.

### Pipe plugins (dmenu mode / rofi script mode)

An action with `"type": "pipe"` turns any script into a dynamic, multi-step
menu — the same protocol as rofi's script mode:

1. **First call** (typing the action name, e.g. `/session`) runs the command
   with no arguments; each stdout line becomes a selectable item. Text after
   the name filters the lines (`/windows fire`).
2. **Round-trip** (selecting a line) re-invokes the same command with the
   selected line as `$1`. If it prints new lines, the menu is replaced
   (multi-step navigation); if it prints nothing, the script performed the
   action itself and the launcher closes.

An action with `"input": "query"` additionally receives the query remainder
as `$1` on the first call (e.g. `/calc 2+2` → `calc.sh "2+2"`); line filtering
is disabled for such actions.

```json
{
    "name": "/calc",
    "type": "pipe",
    "input": "query",
    "command": "@scripts/calc.sh",
    "icon": "accessories-calculator",
    "description": "Evaluate an expression; pick the result to copy it"
}
```

The `@scripts/` prefix resolves bundled scripts from `~/.config/quasar/scripts/`
or the installed `share/quasar/scripts/`. Bundled out of the box:
`calc.sh` (calculator → copies result), `clipboard.sh` (cliphist browser),
`session.sh` (Lock / Log out / Suspend / Reboot / Power off).

### User themes

Built-in themes live in `assets/presets.json`. Drop your own JSON files into
`~/.config/quasar/themes/` — each becomes a selectable preset (the `"name"`
field or the file name is used as the preset name).

### Flatpak

A build manifest is provided in `packaging/com.quasar.launcher.json`
(org.kde.Sdk 6.11):

```sh
flatpak-builder --user --install --install-deps-from=flathub \
    --force-clean build-flat packaging/com.quasar.launcher.json
flatpak run com.quasar.launcher
```

Note: the Flatpak build ships without layer-shell (the sandboxed
layer-shell plugin conflicts with the SDK's Qt platform build — the launcher
runs as a regular frameless always-on-top window there). Native packages
keep the full layer-shell overlay experience on Wayland.

## Debugging

Logging uses Qt categories; enable with:

```sh
QT_LOGGING_RULES="quasar.*.debug=true" quasar
```

## Tests

```sh
ctest --test-dir build --output-on-failure
```
