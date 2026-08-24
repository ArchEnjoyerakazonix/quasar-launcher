# Security Policy

## 🔒 Threat Model & Architecture

Quasar Launcher is designed as an interactive desktop utility operating strictly within the user's unprivileged desktop session. It does not require `root` or `sudo` privileges, runs inside the user's session D-Bus bus, and adheres to the following security design principles:

### 1. D-Bus Interface Hardening & Session Isolation
- **Session Bus Confinement**: `com.quasar.launcher` connects exclusively to `QDBusConnection::sessionBus()`, which listens on the unprivileged per-user socket (`$XDG_RUNTIME_DIR/bus`). The Linux kernel UNIX domain socket permissions restrict access solely to the running user (matching UID), preventing cross-user IPC access on multi-user systems.
- **Minimal Exposed Surface**: Only non-privileged GUI lifecycle slots are exported on `/Main` (`toggle`, `show`, `hide`, `endPreview`).
- **No Remote Code Execution**: No methods accept shell commands or arbitrary executable paths over D-Bus.

### 2. URL & Web Search Sanitization
- All web search queries triggered via `?`, `g:`, `web:`, `b:`, `google:` prefixes are percent-encoded using `QUrl::toPercentEncoding()` before being dispatched to `QDesktopServices::openUrl()` and the default system browser.
- Prevents URI parameter injection and rogue scheme attacks.

### 3. Shell Command Runner & POSIX Single-Quote Escaping
- Commands invoked via the terminal runner (`$`, `>`) are passed to the selected terminal using standard POSIX single-quote escaping:
  ```cpp
  QString quoted(arg);
  quoted.replace(QLatin1Char('\''), QStringLiteral("'\\''"));
  return QLatin1Char('\'') + quoted + QLatin1Char('\'');
  ```
  Every internal `'` is escaped as `'\''`, and the entire command string is wrapped in single quotes, neutralizing command injection regardless of spaces, backticks, `$()`, or nested quotes.
- Child processes have the compositor-specific `QT_WAYLAND_SHELL_INTEGRATION` variable scrubbed via `Platform::detachedStart()` to prevent subprocess layer-shell surface hijacking.

### 4. Mathematical Engine Sandbox & Token Whitelist
- The inline calculator evaluates mathematical expressions using a strict multi-layer defense:
  1. **Character Whitelist**: Only alphanumeric characters, arithmetic operators (`+`, `-`, `*`, `/`, `%`, `^`), parentheses, dots, commas, and whitespace are allowed.
  2. **Token Whitelist**: All identifier tokens in the expression are scanned with regex and strictly checked against a closed whitelist (`sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `sqrt`, `cbrt`, `abs`, `log`, `ln`, `exp`, `round`, `floor`, `ceil`, `pow`, `pi`, `PI`, `e`, `E`, `Math`, `min`, `max`). Any identifier not in this list (e.g. `constructor`, `eval`, `Function`, `globalThis`, `__proto__`, `window`) causes immediate rejection before script evaluation.
  3. **Return Type Enforcement**: The evaluator strictly enforces numeric types (`QJSValue::isNumber()`) and guards against `NaN` and `Infinity`.

### 5. Pipe Plugin Model
- Actions defined in `~/.config/quasar/actions.json` operate with the user's standard permissions.
- Path resolution for `@scripts/` is confined to trusted user configuration and standard system data directories.

---

## 🛡️ Reporting a Vulnerability

If you discover a security vulnerability within Quasar Launcher, please report it responsibly:

1. **Do NOT open a public issue** on GitHub.
2. Submit a private vulnerability report via **GitHub Security Advisories** on the repository, or contact the maintainer directly.
3. Please include:
   - Detailed description of the attack vector.
   - Minimal reproducible proof-of-concept (PoC).
   - Desktop environment and compositor version (Hyprland, Sway, KDE, GNOME, etc.).

We appreciate your contributions to keeping the Linux desktop ecosystem secure.
