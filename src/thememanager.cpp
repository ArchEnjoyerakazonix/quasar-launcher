#include "thememanager.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>

ThemeManager* ThemeManager::instance()
{
    static ThemeManager s_instance;
    return &s_instance;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
{
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &ThemeManager::onFileChanged);
    load();
}

QString ThemeManager::themeFilePath() const
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/quasar";
    return configDir + "/theme.json";
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

    // Ensure watcher monitors the file
    if (!m_watcher.files().contains(path) && QFile::exists(path)) {
        m_watcher.addPath(path);
    }
}

void ThemeManager::onFileChanged(const QString &path)
{
    if (path == themeFilePath()) {
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
    if (obj.contains("promptText")) m_promptText = obj["promptText"].toString();
    if (obj.contains("windowWidth")) m_windowWidth = obj["windowWidth"].toInt();
    if (obj.contains("windowHeight")) m_windowHeight = obj["windowHeight"].toInt();

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
    return obj;
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

QStringList ThemeManager::getAvailablePresets() const
{
    return {
        "Rofi Classic",
        "Catppuccin Macchiato",
        "Nord Dark",
        "Tokyo Night",
        "OLED Black",
        "Modern Glass",
        "Cyberpunk Neon"
    };
}

void ThemeManager::loadPreset(const QString &presetName)
{
    if (presetName == "Rofi Classic") {
        m_layoutMode = "list";
        m_backgroundColor = "#1c1c1c";
        m_bgOpacity = 0.95;
        m_cardColor = "#262626";
        m_cardOpacity = 1.0;
        m_accentColor = "#007acc";
        m_textColor = "#ffffff";
        m_secondaryTextColor = "#888888";
        m_fontFamily = "Monospace";
        m_fontSize = 14;
        m_borderRadius = 4;
        m_borderWidth = 1;
        m_borderColor = "#007acc";
        m_showIcons = true;
        m_iconSize = 24;
        m_promptText = "run: ";
        m_windowWidth = 600;
        m_windowHeight = 400;
    } else if (presetName == "Catppuccin Macchiato") {
        m_layoutMode = "list";
        m_backgroundColor = "#24273a";
        m_bgOpacity = 0.90;
        m_cardColor = "#1e2030";
        m_cardOpacity = 0.95;
        m_accentColor = "#8aadf4";
        m_textColor = "#cad3f5";
        m_secondaryTextColor = "#a5adce";
        m_fontFamily = "Sans";
        m_fontSize = 14;
        m_borderRadius = 12;
        m_borderWidth = 1;
        m_borderColor = "#363a4f";
        m_showIcons = true;
        m_iconSize = 28;
        m_promptText = "dmenu: ";
        m_windowWidth = 640;
        m_windowHeight = 420;
    } else if (presetName == "Nord Dark") {
        m_layoutMode = "list";
        m_backgroundColor = "#2e3440";
        m_bgOpacity = 0.90;
        m_cardColor = "#3b4252";
        m_cardOpacity = 0.95;
        m_accentColor = "#88c0d0";
        m_textColor = "#eceff4";
        m_secondaryTextColor = "#d8dee9";
        m_fontFamily = "Sans";
        m_fontSize = 14;
        m_borderRadius = 8;
        m_borderWidth = 1;
        m_borderColor = "#4c566a";
        m_showIcons = true;
        m_iconSize = 26;
        m_promptText = "find: ";
        m_windowWidth = 620;
        m_windowHeight = 400;
    } else if (presetName == "Tokyo Night") {
        m_layoutMode = "list";
        m_backgroundColor = "#1a1b26";
        m_bgOpacity = 0.92;
        m_cardColor = "#24283b";
        m_cardOpacity = 0.95;
        m_accentColor = "#7aa2f7";
        m_textColor = "#c0caf5";
        m_secondaryTextColor = "#a9b1d6";
        m_fontFamily = "Sans";
        m_fontSize = 14;
        m_borderRadius = 10;
        m_borderWidth = 1;
        m_borderColor = "#414868";
        m_showIcons = true;
        m_iconSize = 28;
        m_promptText = "> ";
        m_windowWidth = 650;
        m_windowHeight = 430;
    } else if (presetName == "OLED Black") {
        m_layoutMode = "list";
        m_backgroundColor = "#000000";
        m_bgOpacity = 1.0;
        m_cardColor = "#111111";
        m_cardOpacity = 1.0;
        m_accentColor = "#00ffcc";
        m_textColor = "#ffffff";
        m_secondaryTextColor = "#777777";
        m_fontFamily = "Sans";
        m_fontSize = 14;
        m_borderRadius = 0;
        m_borderWidth = 2;
        m_borderColor = "#00ffcc";
        m_showIcons = true;
        m_iconSize = 24;
        m_promptText = "apps: ";
        m_windowWidth = 600;
        m_windowHeight = 450;
    } else if (presetName == "Modern Glass") {
        m_layoutMode = "grid";
        m_backgroundColor = "#0a0a14";
        m_bgOpacity = 0.70;
        m_cardColor = "#ffffff";
        m_cardOpacity = 0.10;
        m_accentColor = "#7c3aed";
        m_textColor = "#ffffff";
        m_secondaryTextColor = "#ffffffaa";
        m_fontFamily = "Sans";
        m_fontSize = 15;
        m_borderRadius = 16;
        m_borderWidth = 1;
        m_borderColor = "#ffffff20";
        m_showIcons = true;
        m_iconSize = 48;
        m_promptText = "";
        m_windowWidth = 720;
        m_windowHeight = 500;
    } else if (presetName == "Cyberpunk Neon") {
        m_layoutMode = "list";
        m_backgroundColor = "#0d0221";
        m_bgOpacity = 0.95;
        m_cardColor = "#19053d";
        m_cardOpacity = 0.95;
        m_accentColor = "#ff007f";
        m_textColor = "#00f6ff";
        m_secondaryTextColor = "#ff9900";
        m_fontFamily = "Monospace";
        m_fontSize = 14;
        m_borderRadius = 6;
        m_borderWidth = 2;
        m_borderColor = "#ff007f";
        m_showIcons = true;
        m_iconSize = 28;
        m_promptText = "SYSTEM:// ";
        m_windowWidth = 680;
        m_windowHeight = 440;
    }

    saveTheme();
}

void ThemeManager::setLayoutMode(const QString &v) { if (m_layoutMode != v) { m_layoutMode = v; saveTheme(); } }
void ThemeManager::setBackgroundColor(const QString &v) { if (m_backgroundColor != v) { m_backgroundColor = v; saveTheme(); } }
void ThemeManager::setBgOpacity(double v) { if (m_bgOpacity != v) { m_bgOpacity = v; saveTheme(); } }
void ThemeManager::setCardColor(const QString &v) { if (m_cardColor != v) { m_cardColor = v; saveTheme(); } }
void ThemeManager::setCardOpacity(double v) { if (m_cardOpacity != v) { m_cardOpacity = v; saveTheme(); } }
void ThemeManager::setAccentColor(const QString &v) { if (m_accentColor != v) { m_accentColor = v; saveTheme(); } }
void ThemeManager::setTextColor(const QString &v) { if (m_textColor != v) { m_textColor = v; saveTheme(); } }
void ThemeManager::setSecondaryTextColor(const QString &v) { if (m_secondaryTextColor != v) { m_secondaryTextColor = v; saveTheme(); } }
void ThemeManager::setFontFamily(const QString &v) { if (m_fontFamily != v) { m_fontFamily = v; saveTheme(); } }
void ThemeManager::setFontSize(int v) { if (m_fontSize != v) { m_fontSize = v; saveTheme(); } }
void ThemeManager::setBorderRadius(int v) { if (m_borderRadius != v) { m_borderRadius = v; saveTheme(); } }
void ThemeManager::setBorderWidth(int v) { if (m_borderWidth != v) { m_borderWidth = v; saveTheme(); } }
void ThemeManager::setBorderColor(const QString &v) { if (m_borderColor != v) { m_borderColor = v; saveTheme(); } }
void ThemeManager::setShowIcons(bool v) { if (m_showIcons != v) { m_showIcons = v; saveTheme(); } }
void ThemeManager::setIconSize(int v) { if (m_iconSize != v) { m_iconSize = v; saveTheme(); } }
void ThemeManager::setPromptText(const QString &v) { if (m_promptText != v) { m_promptText = v; saveTheme(); } }
void ThemeManager::setWindowWidth(int v) { if (m_windowWidth != v) { m_windowWidth = v; saveTheme(); } }
void ThemeManager::setWindowHeight(int v) { if (m_windowHeight != v) { m_windowHeight = v; saveTheme(); } }
