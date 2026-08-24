#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QFileSystemWatcher>
#include <QJsonObject>
#include <QMap>
#include <QTimer>

class ThemeManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString layoutMode READ layoutMode WRITE setLayoutMode NOTIFY themeChanged)
    Q_PROPERTY(QString backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY themeChanged)
    Q_PROPERTY(double bgOpacity READ bgOpacity WRITE setBgOpacity NOTIFY themeChanged)
    Q_PROPERTY(QString cardColor READ cardColor WRITE setCardColor NOTIFY themeChanged)
    Q_PROPERTY(double cardOpacity READ cardOpacity WRITE setCardOpacity NOTIFY themeChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY themeChanged)
    Q_PROPERTY(QString textColor READ textColor WRITE setTextColor NOTIFY themeChanged)
    Q_PROPERTY(QString secondaryTextColor READ secondaryTextColor WRITE setSecondaryTextColor NOTIFY themeChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY themeChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY themeChanged)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius NOTIFY themeChanged)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY themeChanged)
    Q_PROPERTY(QString borderColor READ borderColor WRITE setBorderColor NOTIFY themeChanged)
    Q_PROPERTY(bool showIcons READ showIcons WRITE setShowIcons NOTIFY themeChanged)
    Q_PROPERTY(int iconSize READ iconSize WRITE setIconSize NOTIFY themeChanged)
    Q_PROPERTY(QString promptText READ promptText WRITE setPromptText NOTIFY themeChanged)
    Q_PROPERTY(int windowWidth READ windowWidth WRITE setWindowWidth NOTIFY themeChanged)
    Q_PROPERTY(int windowHeight READ windowHeight WRITE setWindowHeight NOTIFY themeChanged)
    Q_PROPERTY(bool enableDimOverlay READ enableDimOverlay WRITE setEnableDimOverlay NOTIFY themeChanged)
    Q_PROPERTY(QString currentPresetName READ currentPresetName NOTIFY themeChanged)
    Q_PROPERTY(QStringList favoritePresets READ favoritePresets NOTIFY favoritesChanged)

public:
    static ThemeManager* instance();
    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager() override;

    QString layoutMode() const { return m_layoutMode; }
    QString backgroundColor() const { return m_backgroundColor; }
    double bgOpacity() const { return m_bgOpacity; }
    QString cardColor() const { return m_cardColor; }
    double cardOpacity() const { return m_cardOpacity; }
    QString accentColor() const { return m_accentColor; }
    QString textColor() const { return m_textColor; }
    QString secondaryTextColor() const { return m_secondaryTextColor; }
    QString fontFamily() const { return m_fontFamily; }
    int fontSize() const { return m_fontSize; }
    int borderRadius() const { return m_borderRadius; }
    int borderWidth() const { return m_borderWidth; }
    QString borderColor() const { return m_borderColor; }
    bool showIcons() const { return m_showIcons; }
    int iconSize() const { return m_iconSize; }
    QString promptText() const { return m_promptText; }
    int windowWidth() const { return m_windowWidth; }
    int windowHeight() const { return m_windowHeight; }
    bool enableDimOverlay() const { return m_enableDimOverlay; }
    // Name of the last applied preset ("" for a hand-tuned theme).
    QString currentPresetName() const { return m_currentPresetName; }

    void setLayoutMode(const QString &v);
    void setBackgroundColor(const QString &v);
    void setBgOpacity(double v);
    void setCardColor(const QString &v);
    void setCardOpacity(double v);
    void setAccentColor(const QString &v);
    void setTextColor(const QString &v);
    void setSecondaryTextColor(const QString &v);
    void setFontFamily(const QString &v);
    void setFontSize(int v);
    void setBorderRadius(int v);
    void setBorderWidth(int v);
    void setBorderColor(const QString &v);
    void setShowIcons(bool v);
    void setIconSize(int v);
    void setPromptText(const QString &v);
    void setWindowWidth(int v);
    void setWindowHeight(int v);
    void setEnableDimOverlay(bool v);

    Q_INVOKABLE void saveTheme();
    Q_INVOKABLE void loadPreset(const QString &presetName);
    Q_INVOKABLE QStringList getAvailablePresets() const;
    Q_INVOKABLE QString getPresetCategory(const QString &presetName) const;
    Q_INVOKABLE QVariantMap getPresetDetails(const QString &presetName) const;
    Q_INVOKABLE bool exportTheme(const QString &filePath);
    Q_INVOKABLE bool importTheme(const QString &filePath);
    Q_INVOKABLE bool syncPywal();

    // Favorites: starred presets pinned to the top of the selector list.
    // Persisted in ~/.config/quasar/favorites.json.
    Q_INVOKABLE bool isFavorite(const QString &presetName) const;
    Q_INVOKABLE void toggleFavorite(const QString &presetName);
    QStringList favoritePresets() const { return m_favorites; }

signals:
    void themeChanged();
    void favoritesChanged();

private slots:
    void onFileChanged(const QString &path);

private:
    void load();
    QString themeFilePath() const;
    QString favoritesFilePath() const;
    void loadFavorites();
    void saveFavorites();
    void applyJsonObject(const QJsonObject &obj);
    QJsonObject toJsonObject() const;
    // Live-tuning writes are batched: many property changes during a single
    // color-picking session produce one disk write after the debounce delay.
    void scheduleSave();

    // Preset registry: name → theme JSON object.
    // Populated from assets/presets.json at startup, then merged with
    // ~/.config/quasar/themes/*.json (user themes override built-in).
    void loadBuiltinPresets();
    void loadUserThemes();
    void addPreset(const QString &name, const QJsonObject &obj);

    QMap<QString, QJsonObject> m_presets;

    QString m_layoutMode = "list";
    QString m_backgroundColor = "#11111b";
    double m_bgOpacity = 0.85;
    QString m_cardColor = "#1e1e2e";
    double m_cardOpacity = 0.90;
    QString m_accentColor = "#89b4fa";
    QString m_textColor = "#cdd6f4";
    QString m_secondaryTextColor = "#a6adc8";
    QString m_fontFamily = "Sans";
    int m_fontSize = 14;
    int m_borderRadius = 10;
    int m_borderWidth = 1;
    QString m_borderColor = "#313244";
    bool m_showIcons = true;
    int m_iconSize = 28;
    QString m_promptText = "run: ";
    int m_windowWidth = 650;
    int m_windowHeight = 420;
    bool m_enableDimOverlay = false;
    QString m_currentPresetName;
    QStringList m_favorites;

    QFileSystemWatcher m_watcher;
    QTimer m_saveDebounce;
};
