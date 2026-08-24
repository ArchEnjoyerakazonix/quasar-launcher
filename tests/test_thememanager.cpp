#include <QtTest>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "../src/thememanager.h"

class TestThemeManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        QStandardPaths::setTestModeEnabled(true);
    }

    void init() {
        // Each test starts from a clean state file so ordering never leaks.
        QFile::remove(Platform_configDir() + "/theme.json");
    }

    void builtinPresetsLoaded() {
        ThemeManager tm;
        const QStringList presets = tm.getAvailablePresets();
        QVERIFY(presets.size() >= 33);
        QVERIFY(presets.contains("Catppuccin Mocha"));
        QVERIFY(presets.contains("Classic Minimal"));
        QVERIFY(presets.contains("Modern Glass"));
    }

    void loadPresetAppliesValues() {
        ThemeManager tm;
        tm.loadPreset("Catppuccin Mocha");
        QCOMPARE(tm.accentColor(), QStringLiteral("#cba6f7"));
        QCOMPARE(tm.borderRadius(), 14);
        QCOMPARE(tm.layoutMode(), QStringLiteral("list"));
        QCOMPARE(tm.textColor(), QStringLiteral("#cdd6f4"));

        // Presets restyle colors only — the layout mode stays the user's
        // choice, so any theme works in any mode.
        tm.setLayoutMode("grid");
        tm.loadPreset("Modern Glass");
        QCOMPARE(tm.bgOpacity(), 0.70);
        QCOMPARE(tm.layoutMode(), QStringLiteral("grid"));

        tm.loadPreset("OLED Black");
        QCOMPARE(tm.backgroundColor(), QStringLiteral("#000000"));
        QCOMPARE(tm.borderRadius(), 0);
        QCOMPARE(tm.layoutMode(), QStringLiteral("grid"));
    }

    void presetCategories() {
        ThemeManager tm;
        QCOMPARE(tm.getPresetCategory("Catppuccin Latte"), QStringLiteral("Light"));
        QCOMPARE(tm.getPresetCategory("Gruvbox Light"), QStringLiteral("Light"));
        QCOMPARE(tm.getPresetCategory("Catppuccin Mocha"), QStringLiteral("Dark"));
        QCOMPARE(tm.getPresetCategory("Classic Minimal"), QStringLiteral("Retro"));
        QCOMPARE(tm.getPresetCategory("Cyberpunk Neon"), QStringLiteral("Neon"));
        QCOMPARE(tm.getPresetCategory("Synthwave '84"), QStringLiteral("Neon"));
    }

    void unknownPresetDoesNotCrash() {
        ThemeManager tm;
        tm.loadPreset("Definitely Not A Preset");
        QCOMPARE(tm.accentColor(), QStringLiteral("#89b4fa")); // default kept
    }

    void userThemesLoaded() {
        const QString themesDir = Platform_configDir() + "/themes";
        QDir().mkpath(themesDir);
        QFile f(themesDir + "/mytheme.json");
        QVERIFY(f.open(QIODevice::WriteOnly));
        // NOTE: keep plain string literals — moc cannot parse raw strings
        // (R"(...)") and silently skips the whole file's Q_OBJECT classes.
        f.write("{\"name\": \"My Custom Theme\", \"accentColor\": \"#ff0000\", "
                "\"backgroundColor\": \"#0a0a0a\", \"borderRadius\": 20}");
        f.close();

        ThemeManager tm;
        QVERIFY(tm.getAvailablePresets().contains("My Custom Theme"));

        tm.loadPreset("My Custom Theme");
        QCOMPARE(tm.accentColor(), QStringLiteral("#ff0000"));
        QCOMPARE(tm.borderRadius(), 20);

        // Explicit "category" wins over the lightness heuristic.
        QCOMPARE(tm.getPresetCategory("My Custom Theme"), QStringLiteral("Dark"));

        f.remove();
    }

    void debouncedSaveWritesOnce() {
        ThemeManager tm;
        QSignalSpy changedSpy(&tm, &ThemeManager::themeChanged);

        // A burst of live-tuning changes…
        tm.setAccentColor("#111111");
        tm.setBackgroundColor("#222222");
        tm.setTextColor("#333333");
        tm.setBorderRadius(7);

        // …must not hit the disk immediately.
        QFile f(Platform_configDir() + "/theme.json");
        QVERIFY(!f.exists());

        // Wait past the 300 ms debounce window.
        QTest::qWait(450);
        QVERIFY(f.exists());

        QJsonDocument doc = QJsonDocument::fromJson(fileContents(f));
        QVERIFY(doc.isObject());
        QCOMPARE(doc.object()["accentColor"].toString(), QStringLiteral("#111111"));
        QCOMPARE(doc.object()["borderRadius"].toInt(), 7);
    }

    void currentPresetNamePersists() {
        {
            ThemeManager tm;
            tm.loadPreset("Dracula");
            QCOMPARE(tm.currentPresetName(), QStringLiteral("Dracula"));
        }
        // A fresh instance must restore the applied preset name from disk and
        // not reset the selection to the first list entry.
        ThemeManager tm2;
        QCOMPARE(tm2.currentPresetName(), QStringLiteral("Dracula"));
    }

    void favoritesPersistAcrossInstances() {
        QFile::remove(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                      + "/quasar/favorites.json");
        {
            ThemeManager tm;
            QVERIFY(!tm.isFavorite("Dracula"));
            tm.toggleFavorite("Dracula");
            tm.toggleFavorite("Nord Dark");
            QVERIFY(tm.isFavorite("Dracula"));
            QVERIFY(tm.isFavorite("Nord Dark"));
            QCOMPARE(tm.favoritePresets(),
                     QStringList({QStringLiteral("Dracula"), QStringLiteral("Nord Dark")}));
            // Toggling again removes
            tm.toggleFavorite("Dracula");
            QVERIFY(!tm.isFavorite("Dracula"));
            tm.toggleFavorite("Dracula");
        }
        ThemeManager tm2;
        QVERIFY(tm2.isFavorite("Dracula"));
        QVERIFY(tm2.isFavorite("Nord Dark"));
    }

    void importExportRoundTrip() {
        ThemeManager tm;
        tm.loadPreset("Dracula");
        const QString tmp = QDir::temp().filePath("quasar_theme_export.json");
        QVERIFY(tm.exportTheme(tmp));

        ThemeManager tm2;
        QVERIFY(tm2.importTheme(tmp));
        QCOMPARE(tm2.accentColor(), tm.accentColor());
        QCOMPARE(tm2.textColor(), tm.textColor());
        QFile::remove(tmp);
    }

private:
    static QString Platform_configDir() {
        return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
               + "/quasar";
    }

    static QByteArray fileContents(QFile &f) {
        f.open(QIODevice::ReadOnly);
        const QByteArray data = f.readAll();
        f.close();
        return data;
    }
};

QTEST_MAIN(TestThemeManager)
#include "test_thememanager.moc"
