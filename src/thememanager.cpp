#include "thememanager.h"
#include "platform.h"
#include "logging.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QColor>
#include <QCoreApplication>

ThemeManager* ThemeManager::instance()
{
    static ThemeManager s_instance;
    return &s_instance;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &ThemeManager::onFileChanged);

    m_saveDebounce.setSingleShot(true);
    m_saveDebounce.setInterval(300);
    connect(&m_saveDebounce, &QTimer::timeout, this, &ThemeManager::saveTheme);

    loadBuiltinPresets();
    loadUserThemes();
    loadFavorites();
    load();
}

ThemeManager::~ThemeManager()
{
    // Flush a pending debounced write so a quick close never loses a tweak.
    if (m_saveDebounce.isActive()) {
        m_saveDebounce.stop();
        saveTheme();
    }
}

QString ThemeManager::themeFilePath() const
{
    return Platform::configDir() + "/theme.json";
}

QString ThemeManager::favoritesFilePath() const
{
    return Platform::configDir() + "/favorites.json";
}

void ThemeManager::loadFavorites()
{
    QFile file(favoritesFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isArray()) {
        m_favorites.clear();
        for (const QJsonValue &v : doc.array()) {
            const QString name = v.toString();
            if (!name.isEmpty() && !m_favorites.contains(name)) {
                m_favorites.append(name);
            }
        }
    }
}

void ThemeManager::saveFavorites()
{
    QFileInfo fi(favoritesFilePath());
    QDir().mkpath(fi.absolutePath());
    QFile file(favoritesFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        qCWarning(lcLauncher) << "cannot write favorites file";
        return;
    }
    file.write(QJsonDocument(QJsonArray::fromStringList(m_favorites)).toJson());
}

bool ThemeManager::isFavorite(const QString &presetName) const
{
    return m_favorites.contains(presetName);
}

void ThemeManager::toggleFavorite(const QString &presetName)
{
    if (presetName.isEmpty()) {
        return;
    }
    if (m_favorites.contains(presetName)) {
        m_favorites.removeAll(presetName);
    } else {
        m_favorites.append(presetName);
    }
    saveFavorites();
    emit favoritesChanged();
}

void ThemeManager::load()
{
    QString path = themeFilePath();
    QFile file(path);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject()) {
            applyJsonObject(doc.object());
        }
        file.close();
    }

    if (!m_watcher.files().contains(path) && QFile::exists(path)) {
        m_watcher.addPath(path);
    }
}

void ThemeManager::onFileChanged(const QString &path)
{
    if (path == themeFilePath()) {
        qCDebug(lcLauncher) << "theme.json changed on disk — live-reloading";
        load();
        emit themeChanged();
    }
}

void ThemeManager::applyJsonObject(const QJsonObject &obj)
{
    if (obj.contains("layoutMode")) m_layoutMode = obj["layoutMode"].toString();
    if (obj.contains("backgroundColor")) m_backgroundColor = obj["backgroundColor"].toString();
    if (obj.contains("bgOpacity")) m_bgOpacity = obj["bgOpacity"].toDouble();
    if (obj.contains("cardColor")) m_cardColor = obj["cardColor"].toString();
    if (obj.contains("cardOpacity")) m_cardOpacity = obj["cardOpacity"].toDouble();
    if (obj.contains("accentColor")) m_accentColor = obj["accentColor"].toString();
    if (obj.contains("textColor")) m_textColor = obj["textColor"].toString();
    if (obj.contains("secondaryTextColor")) m_secondaryTextColor = obj["secondaryTextColor"].toString();
    if (obj.contains("fontFamily")) m_fontFamily = obj["fontFamily"].toString();
    if (obj.contains("fontSize")) m_fontSize = obj["fontSize"].toInt();
    if (obj.contains("borderRadius")) m_borderRadius = obj["borderRadius"].toInt();
    if (obj.contains("borderWidth")) m_borderWidth = obj["borderWidth"].toInt();
    if (obj.contains("borderColor")) m_borderColor = obj["borderColor"].toString();
    if (obj.contains("showIcons")) m_showIcons = obj["showIcons"].toBool();
    if (obj.contains("iconSize")) m_iconSize = obj["iconSize"].toInt();
    m_promptText = QString();
    if (obj.contains("windowWidth")) m_windowWidth = obj["windowWidth"].toInt();
    if (obj.contains("windowHeight")) m_windowHeight = obj["windowHeight"].toInt();
    if (obj.contains("enableDimOverlay")) m_enableDimOverlay = obj["enableDimOverlay"].toBool();
    if (obj.contains("presetName")) m_currentPresetName = obj["presetName"].toString();

    emit themeChanged();
}

QJsonObject ThemeManager::toJsonObject() const
{
    QJsonObject obj;
    obj["layoutMode"] = m_layoutMode;
    obj["backgroundColor"] = m_backgroundColor;
    obj["bgOpacity"] = m_bgOpacity;
    obj["cardColor"] = m_cardColor;
    obj["cardOpacity"] = m_cardOpacity;
    obj["accentColor"] = m_accentColor;
    obj["textColor"] = m_textColor;
    obj["secondaryTextColor"] = m_secondaryTextColor;
    obj["fontFamily"] = m_fontFamily;
    obj["fontSize"] = m_fontSize;
    obj["borderRadius"] = m_borderRadius;
    obj["borderWidth"] = m_borderWidth;
    obj["borderColor"] = m_borderColor;
    obj["showIcons"] = m_showIcons;
    obj["iconSize"] = m_iconSize;
    obj["promptText"] = m_promptText;
    obj["windowWidth"] = m_windowWidth;
    obj["windowHeight"] = m_windowHeight;
    obj["enableDimOverlay"] = m_enableDimOverlay;
    obj["presetName"] = m_currentPresetName;
    return obj;
}

void ThemeManager::scheduleSave()
{
    m_saveDebounce.start();
}

void ThemeManager::saveTheme()
{
    QString path = themeFilePath();
    QFileInfo fi(path);
    QDir().mkpath(fi.absolutePath());

    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(toJsonObject());
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }

    if (!m_watcher.files().contains(path)) {
        m_watcher.addPath(path);
    }
    emit themeChanged();
}

// ---------------------------------------------------------------------------
// Preset registry: built-in (bundled JSON) + user themes (~/.config/quasar/themes/)
// ---------------------------------------------------------------------------

void ThemeManager::loadBuiltinPresets()
{
    // Try the compiled Qt resource first (qrc://), then the installed and
    // build-tree locations (tests run from the build dir).
    QJsonDocument doc;
    const QStringList candidates = {
        QStringLiteral(":com/quasar/launcher/assets/presets.json"),
        QStringLiteral(":com/quasar/themeselector/assets/presets.json"),
        qEnvironmentVariable("QUASAR_PRESETS"),
        QStringLiteral("assets/presets.json"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../assets/presets.json"),
        QCoreApplication::applicationDirPath() + QStringLiteral("/../share/quasar/presets.json"),
        QStringLiteral("/usr/local/share/quasar/presets.json"),
        QStringLiteral("/usr/share/quasar/presets.json"),
    };
    for (const QString &candidate : candidates) {
        if (candidate.isEmpty()) continue;
        QFile f(candidate);
        if (f.open(QIODevice::ReadOnly)) {
            doc = QJsonDocument::fromJson(f.readAll());
            if (doc.isObject()) break;
        }
    }

    if (!doc.isObject()) {
        qCWarning(lcLauncher) << "failed to load built-in presets";
        return;
    }

    const QJsonObject presets = doc.object();
    for (auto it = presets.constBegin(); it != presets.constEnd(); ++it) {
        if (it.value().isObject()) {
            addPreset(it.key(), it.value().toObject());
        }
    }
    qCDebug(lcLauncher) << "loaded" << m_presets.size() << "built-in presets";
}

void ThemeManager::loadUserThemes()
{
    const QString themesDir = Platform::configDir() + "/themes";
    QDir dir(themesDir);
    if (!dir.exists()) {
        dir.mkpath(".");
        return;
    }

    int count = 0;
    for (const QFileInfo &fi : dir.entryInfoList({"*.json"}, QDir::Files)) {
        QFile f(fi.absoluteFilePath());
        if (!f.open(QIODevice::ReadOnly)) continue;
        QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
        if (!doc.isObject()) continue;

        // A user theme JSON must contain at least "name" + theme properties.
        QString name = doc.object()["name"].toString();
        if (name.isEmpty()) {
            name = fi.baseName();
        }
        addPreset(name, doc.object());
        m_watcher.addPath(fi.absoluteFilePath());
        ++count;
    }

    if (count > 0) {
        qCDebug(lcLauncher) << "loaded" << count << "user themes from" << themesDir;
    }
}

void ThemeManager::addPreset(const QString &name, const QJsonObject &obj)
{
    m_presets.insert(name, obj);
}

// ---------------------------------------------------------------------------
// Public preset API (unchanged signatures)
// ---------------------------------------------------------------------------

void ThemeManager::loadPreset(const QString &presetName)
{
    auto it = m_presets.constFind(presetName);
    if (it != m_presets.constEnd()) {
        m_currentPresetName = presetName;
        // The layout mode (list/grid/compact) is the user's own choice made
        // with the mode pills — presets only restyle colors/fonts/geometry,
        // so any theme can be used in any mode.
        QJsonObject preset = it.value();
        preset.remove(QStringLiteral("layoutMode"));
        applyJsonObject(preset);
        saveTheme();
        qCDebug(lcLauncher) << "applied preset (colors only):" << presetName;
    } else {
        qCWarning(lcLauncher) << "preset not found:" << presetName;
    }
}

QStringList ThemeManager::getAvailablePresets() const
{
    return m_presets.keys();
}

QString ThemeManager::getPresetCategory(const QString &presetName) const
{
    auto it = m_presets.constFind(presetName);
    if (it == m_presets.constEnd()) return {};

    const QJsonObject obj = it.value();

    // Heuristic: dark = low-luminance background, light = high-luminance.
    // A user theme may set "category" explicitly.
    if (obj.contains("category"))
        return obj["category"].toString();

    const QColor bg(obj["backgroundColor"].toString());
    if (bg.isValid() && bg.lightness() > 190)
        return QStringLiteral("Light");

    // Neon: saturated accent or futuristic / cyber styling
    if (presetName.contains("Cyber") || presetName.contains("Neon") || presetName.contains("Synthwave") 
        || presetName.contains("Outrun") || presetName.contains("Glass") || presetName.contains("Matrix")
        || presetName.contains("Hyper") || presetName.contains("Blade Runner")
        || presetName.contains("Void") || presetName.contains("Imperial") || presetName.contains("Shadow")
        || presetName.contains("Genesis") || presetName.contains("OLED")
        || presetName.contains("Emerald") || presetName.contains("Sapphire") || presetName.contains("Ruby")
        || presetName.contains("Amethyst") || presetName.contains("Crimson")) {
        return QStringLiteral("Neon");
    }
    if (presetName.startsWith("Rofi")) return QStringLiteral("Retro");

    return QStringLiteral("Dark");
}

QVariantMap ThemeManager::getPresetDetails(const QString &presetName) const
{
    auto it = m_presets.constFind(presetName);
    if (it == m_presets.constEnd()) return {};

    const QJsonObject obj = it.value();
    QVariantMap map;
    map["backgroundColor"] = obj["backgroundColor"].toString();
    map["accentColor"] = obj["accentColor"].toString();
    map["cardColor"] = obj["cardColor"].toString();
    map["textColor"] = obj["textColor"].toString();
    map["secondaryTextColor"] = obj["secondaryTextColor"].toString();
    map["promptText"] = obj["promptText"].toString();
    map["layoutMode"] = obj["layoutMode"].toString();
    return map;
}

bool ThemeManager::exportTheme(const QString &filePath)
{
    QString cleanPath = filePath;
    if (cleanPath.startsWith("file://")) {
        cleanPath = QUrl(cleanPath).toLocalFile();
    }
    QFile file(cleanPath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    QJsonDocument doc(toJsonObject());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool ThemeManager::importTheme(const QString &filePath)
{
    QString cleanPath = filePath;
    if (cleanPath.startsWith("file://")) {
        cleanPath = QUrl(cleanPath).toLocalFile();
    }
    QFile file(cleanPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) {
        return false;
    }
    applyJsonObject(doc.object());
    saveTheme();
    return true;
}

bool ThemeManager::syncPywal()
{
    const QString walColorsPath = QDir::homePath() + QStringLiteral("/.cache/wal/colors.json");
    QFile file(walColorsPath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        qCDebug(lcLauncher) << "Pywal colors.json not found at" << walColorsPath;
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    QJsonObject special = root.value(QLatin1String("special")).toObject();
    QJsonObject colors = root.value(QLatin1String("colors")).toObject();

    QString bg = special.value(QLatin1String("background")).toString("#11111b");
    QString fg = special.value(QLatin1String("foreground")).toString("#cdd6f4");
    QString accent = colors.value(QLatin1String("color4")).toString("#89b4fa");
    QString card = colors.value(QLatin1String("color0")).toString("#1e1e2e");
    QString border = colors.value(QLatin1String("color8")).toString("#313244");
    QString secondary = colors.value(QLatin1String("color7")).toString("#a6adc8");

    QJsonObject pywalPreset;
    pywalPreset[QLatin1String("name")] = QStringLiteral("Pywal (Auto)");
    pywalPreset[QLatin1String("category")] = QStringLiteral("Dynamic");
    pywalPreset[QLatin1String("backgroundColor")] = bg;
    pywalPreset[QLatin1String("bgOpacity")] = 0.85;
    pywalPreset[QLatin1String("cardColor")] = card;
    pywalPreset[QLatin1String("cardOpacity")] = 0.90;
    pywalPreset[QLatin1String("accentColor")] = accent;
    pywalPreset[QLatin1String("textColor")] = fg;
    pywalPreset[QLatin1String("secondaryTextColor")] = secondary;
    pywalPreset[QLatin1String("borderColor")] = border;
    pywalPreset[QLatin1String("borderRadius")] = 12;
    pywalPreset[QLatin1String("borderWidth")] = 1;

    addPreset(QStringLiteral("Pywal (Auto)"), pywalPreset);
    applyJsonObject(pywalPreset);
    saveTheme();
    return true;
}

void ThemeManager::setLayoutMode(const QString &v) { if (m_layoutMode != v) { m_layoutMode = v; scheduleSave(); } }
void ThemeManager::setBackgroundColor(const QString &v) { if (m_backgroundColor != v) { m_backgroundColor = v; scheduleSave(); } }
void ThemeManager::setBgOpacity(double v) { if (m_bgOpacity != v) { m_bgOpacity = v; scheduleSave(); } }
void ThemeManager::setCardColor(const QString &v) { if (m_cardColor != v) { m_cardColor = v; scheduleSave(); } }
void ThemeManager::setCardOpacity(double v) { if (m_cardOpacity != v) { m_cardOpacity = v; scheduleSave(); } }
void ThemeManager::setAccentColor(const QString &v) { if (m_accentColor != v) { m_accentColor = v; scheduleSave(); } }
void ThemeManager::setTextColor(const QString &v) { if (m_textColor != v) { m_textColor = v; scheduleSave(); } }
void ThemeManager::setSecondaryTextColor(const QString &v) { if (m_secondaryTextColor != v) { m_secondaryTextColor = v; scheduleSave(); } }
void ThemeManager::setFontFamily(const QString &v) { if (m_fontFamily != v) { m_fontFamily = v; scheduleSave(); } }
void ThemeManager::setFontSize(int v) { if (m_fontSize != v) { m_fontSize = v; scheduleSave(); } }
void ThemeManager::setBorderRadius(int v) { if (m_borderRadius != v) { m_borderRadius = v; scheduleSave(); } }
void ThemeManager::setBorderWidth(int v) { if (m_borderWidth != v) { m_borderWidth = v; scheduleSave(); } }
void ThemeManager::setBorderColor(const QString &v) { if (m_borderColor != v) { m_borderColor = v; scheduleSave(); } }
void ThemeManager::setShowIcons(bool v) { if (m_showIcons != v) { m_showIcons = v; scheduleSave(); } }
void ThemeManager::setIconSize(int v) { if (m_iconSize != v) { m_iconSize = v; scheduleSave(); } }
void ThemeManager::setPromptText(const QString &v) { if (m_promptText != v) { m_promptText = v; scheduleSave(); } }
void ThemeManager::setWindowWidth(int v) { if (m_windowWidth != v) { m_windowWidth = v; scheduleSave(); } }
void ThemeManager::setWindowHeight(int v) { if (m_windowHeight != v) { m_windowHeight = v; scheduleSave(); } }
void ThemeManager::setEnableDimOverlay(bool v) { if (m_enableDimOverlay != v) { m_enableDimOverlay = v; scheduleSave(); } }
