# Security Policy

## 🔒 Threat Model & Architecture

Quasar Launcher is designed as an interactive desktop utility operating strictly within the user's unprivileged desktop session. It does not require `root` or `sudo` privileges, runs inside the user's session D-Bus bus, and adheres to the following security design principles:

### 1. D-Bus Interface Hardening
- **Service Name**: `com.quasar.launcher` registered on `QDBusConnection::sessionBus()`.
- **Exposed Surface**: Only non-privileged, idempotent GUI lifecycle slots are exported on `/Main` (`toggle`, `show`, `hide`, `endPreview`).
- **No Remote Method Invocation**: Arbitrary code execution or system parameter modifications cannot be triggered via D-Bus methods.

### 2. URL & Web Search Sanitization
- All web search queries triggered via `?`, `g:`, `web:`, `b:`, `google:` prefixes or mouse clicks are strictly percent-encoded using `QUrl::toPercentEncoding()` before being dispatched to `QDesktopServices::openUrl()` and the system default browser.
- Prevents URL parameter injection and malformed URI scheme attacks.

### 3. Shell Command Runner & Subprocess Isolation
- Commands invoked via the terminal runner (`$`, `>`) are escaped using POSIX compliant single-quote wrapping (`'\\''`) before wrapping inside the detected terminal emulator.
- Child processes have the compositor-specific `QT_WAYLAND_SHELL_INTEGRATION` variable scrubbed via `Platform::detachedStart()` to prevent subprocess layer-shell surface hijacking.

### 4. Mathematical Engine Sandbox
- The inline calculator evaluates mathematical expressions using a strict multi-layer defense:
  1. **Character Whitelist Regex**: Only arithmetic tokens, math functions (`sin`, `cos`, `tan`, `sqrt`, `cbrt`, `log`, `ln`, `pow`, `pi`, `abs`), digits, and parentheses are allowed.
  2. **Validation**: Submissions matching dangerous JavaScript identifiers, object access, constructor tampering, or network sockets are rejected immediately without evaluation.
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
