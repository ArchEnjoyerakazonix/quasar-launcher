<div align="center">

# ✦ QUASAR LAUNCHER ✦

**Next-generation, ultra-fast application launcher, command palette, and desktop intelligence hub for Wayland & X11.**

[![CI](https://github.com/ArchEnjoyerakazonix/quasar-launcher/actions/workflows/ci.yml/badge.svg)](https://github.com/ArchEnjoyerakazonix/quasar-launcher/actions/workflows/ci.yml)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/20)
[![Qt6](https://img.shields.io/badge/Qt-6.6+-41CD52?style=flat-square&logo=qt)](https://www.qt.io/)
[![Wayland](https://img.shields.io/badge/Wayland-Layer--Shell-brightgreen?style=flat-square&logo=wayland)](https://wayland.freedesktop.org/)
[![X11](https://img.shields.io/badge/X11-EWMH-blue?style=flat-square&logo=xorg)](https://www.x.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square)](https://opensource.org/licenses/MIT)

<br/><br/>

<img src="assets/screenshots/preview.gif" alt="Quasar Launcher Live Preview" width="800"/>

</div>

---

## Overview

**Quasar** is a modern, zero-config application launcher and command palette built with **C++20** and **Qt6 / QML**. Designed to replace legacy, complex launchers with a fluid, glassmorphic experience that works out of the box with zero manual configuration.

Whether you run **Hyprland**, **Sway**, **KDE Plasma**, **GNOME**, or **XFCE**, Quasar natively adapts to your desktop compositor with smooth animations, hardware-accelerated rendering, and comprehensive subsystem integrations.

---

## Features

- **Instant Fuzzy Matching**: Powered by Damerau-Levenshtein distance matching and intelligent Cyrillic $\leftrightarrow$ QWERTY layout auto-translation (e.g. typing `stema` or `сркщьу` automatically matches *Google Chrome*).
- **Frecency Ranking**: Smart scoring that tracks application launch frequency and recency, putting your most-used apps at the top without cluttering the view.
- **Live Inline Math Calculator**: Evaluate expressions in real time (`125 * 8`, `sqrt(256)`, `(50 + 25) / 3`, `2^10`, `sin(pi/2)`) — press <kbd>Enter</kbd> to copy the formatted result directly to your clipboard.
- **Integrated Clipboard Manager**: Browse, search, and restore clipboard history (`c.`) with live color chip previews, powered by `cliphist` on Wayland and `QClipboard` on X11.
- **Emoji & Symbol Picker**: Fast search across 1800+ Unicode emojis and symbols with tags (`e.fire`, `emoji:rocket`, `:cat`) and instant copy.
- **Active Window Switcher**: Switch windows across all workspaces (`w.`, `w:`, `window:`) with native IPC for Hyprland (`hyprctl`), Sway (`swaymsg`), and X11 (`wmctrl`).
- **73 Designer Themes & Live Customizer**: Built-in glassmorphism, cyberpunk neon, retro terminal, and minimal palettes. Tune accent colors, opacity, blur, and borders in real-time with `quasar-theme-selector`.
- **Dynamic Pywal Wallpaper Sync**: Instantly harmonize Quasar colors with your current desktop wallpaper on the fly.
- **Smart Command Runner**: Execute shell commands (`$`, `>`) in your preferred terminal emulator (auto-detects `ghostty`, `kitty`, `alacritty`, `foot`, `wezterm`, `konsole`, `gnome-terminal`, etc.).
- **Web Search Integration**: Query the web with default browser routing (`?`, `g:`, `web:`) via standard FreeDesktop / XDG handlers.
- **Extensible Slash Actions & Pipe Plugins**: Custom JSON actions and dmenu-compatible pipe scripts (`/`) for custom workflows (session management, password store, screenshot hubs).
- **Single-Instance D-Bus Daemon**: Sub-millisecond toggle responsiveness via memory-resident D-Bus IPC service (`quasar --toggle`).

---

## Shortcuts & Query Prefixes

Quasar uses intuitive prefix shortcuts to switch modes seamlessly without needing separate keybinds:

| Prefix | Mode | Example | Description |
|---|---|---|---|
| *(none)* | **App Search & Math** | `firefox`, `stema`, `125 * 8`, `sqrt(144)` | Fuzzy app finder and real-time inline calculator |
| `c.` / `clip.` / `cb.` | **Clipboard Manager** | `c.`, `c.token`, `clip:url` | Search clipboard history with color & text previews |
| `e.` / `emoji.` / `:` | **Emoji Picker** | `e.fire`, `emoji:rocket`, `:tux` | Search 1800+ emojis and copy to clipboard |
| `w.` / `w:` / `window:` | **Window Switcher** | `w.code`, `w:browser`, `window:term` | Focus open windows across workspaces |
| `/` | **Slash Actions & Plugins** | `/screenshot`, `/pass`, `/myaction` | Trigger user-defined actions and dmenu pipe scripts |
| `$` / `>` | **Shell Command** | `$btop`, `>ping -c 3 archlinux.org` | Run commands in your default terminal |
| `?` / `g:` / `web:` | **Web Search** | `?arch wiki`, `g:rust docs`, `web:github` | Open search queries in your default browser |

---

## Installation

### 1. Arch Linux / Manjaro / EndeavourOS

#### Local PKGBUILD:
```bash
git clone https://github.com/ArchEnjoyerakazonix/quasar-launcher.git
cd quasar-launcher/packaging
makepkg -si
```

---

### 2. Building from Source (All Distributions)

#### Dependencies:
- **C++20 compiler** (`gcc` 11+ or `clang` 14+)
- **CMake** 3.20+ & **Ninja**
- **Qt6**: `qt6-base`, `qt6-declarative`, `qt6-svg`
- *(Optional on Wayland)*: `layer-shell-qt` (for native Wayland overlay positioning)

#### Build commands:
```bash
git clone https://github.com/ArchEnjoyerakazonix/quasar-launcher.git
cd quasar-launcher

# Configure & compile
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j$(nproc)

# Install
sudo cmake --install build
```

---

## Desktop & Window Manager Integration

Bind `quasar --toggle` to your preferred hotkey:

### Hyprland (`~/.config/hypr/hyprland.conf`):
```ini
# Toggle Quasar Launcher
bind = $mainMod, SPACE, exec, quasar --toggle

# Toggle Quasar Theme Selector
bind = $mainMod SHIFT, T, exec, quasar-theme-selector
```

### Sway (`~/.config/sway/config`):
```ini
bindsym $mod+Space exec quasar --toggle
bindsym $mod+Shift+t exec quasar-theme-selector
```

### i3 / X11 (`~/.config/i3/config`):
```ini
bindsym $mod+space exec --no-startup-id quasar --toggle
```

### KDE Plasma / GNOME:
Set a custom global shortcut in **Settings $\rightarrow$ Keyboard $\rightarrow$ Shortcuts** pointing to `quasar --toggle`.

---

## Theming & Customization

### Interactive Theme Selector
Launch `quasar-theme-selector` or configure custom shortcuts to open the live visual customizer:
- Browse **73 handcrafted presets** (Cyberpunk Neon, Tokyo Night, Catppuccin Mocha, Dracula, Monokai, Nord, Gruvbox, Solarized, Paper Minimal, etc.).
- Fine-tune window width, height, border radius, background opacity, card glow, font family, and font size in real time.
- All configurations are saved cleanly in `~/.config/quasar/theme.json`.

### Dynamic Wallpaper Sync (Pywal / Matugen)
If you use `pywal` or generate color schemes in `~/.cache/wal/colors.json`, run `quasar --sync-wal` to sync its entire palette to your current wallpaper instantly.

---

## Custom Actions & Pipe Plugins

You can define custom slash commands and dmenu-style pipe scripts in `~/.config/quasar/actions.json`:

```json
[
    {
        "name": "/screenshot",
        "command": "grimblast copy area",
        "icon": "camera-photo",
        "description": "Capture region screenshot to clipboard"
    },
    {
        "name": "/pass",
        "type": "pipe",
        "command": "cd ~/.local/share/password-store && find . -name '*.gpg' | sed 's|.*/||; s|\\.gpg$||'",
        "icon": "dialog-password",
        "description": "Browse password store"
    }
]
```

---

## Testing & Quality Assurance

Quasar comes with a comprehensive CTest suite covering fuzzy ranking, keyboard layout translation, mathematics evaluation, and theme management:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## Security

For threat models, IPC isolation details, and responsible vulnerability reporting, please see [SECURITY.md](SECURITY.md).

---

## License

Quasar is distributed under the terms of the **MIT License**. See [LICENSE](LICENSE) for details.
