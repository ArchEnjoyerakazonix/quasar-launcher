#include <QGuiApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QCommandLineParser>
#include <QWindow>
#include <QIcon>
#include <QQuickImageProvider>
#include <QPixmap>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QMutex>
#include <QMutexLocker>

#ifdef QUASAR_HAVE_LAYERSHELL
#include <LayerShellQt/window.h>
#include <LayerShellQt/shell.h>
#endif

#include "frecencyranker.h"
#include "appindexer.h"
#include "fuzzymatcher.h"
#include "thememanager.h"
#include "launchercontroller.h"
#include "windowswitcher.h"
#include "actionmodel.h"
#include "emojimanager.h"
#include "clipboardmanager.h"
#include "platform.h"
#include "logging.h"

#include <QPixmapCache>
#include <QSet>
#include <QScreen>
#include <QCursor>

namespace {

bool runningOnWayland()
{
    return qEnvironmentVariableIsSet("WAYLAND_DISPLAY");
}

// Centers a frameless window on the screen the cursor is on. Used on X11,
// where the layer-shell placement (Wayland) is not available.
void centerOnCursorScreen(QWindow *window)
{
    QScreen *cursorScreen = QGuiApplication::screenAt(QCursor::pos());
    if (!cursorScreen) {
        cursorScreen = QGuiApplication::primaryScreen();
    }
    if (!cursorScreen || !window) {
        return;
    }
    window->setScreen(cursorScreen);
    const QRect geo = cursorScreen->availableGeometry();
    window->setFramePosition(geo.center() - QPoint(window->width() / 2, window->height() / 2));
}

void configureLayerShell(QWindow *window)
{
#ifdef QUASAR_HAVE_LAYERSHELL
    LayerShellQt::Window *lsWindow = LayerShellQt::Window::get(window);
    if (lsWindow) {
        lsWindow->setLayer(LayerShellQt::Window::LayerTop);
        lsWindow->setAnchors(LayerShellQt::Window::Anchors(0));
        lsWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityExclusive);
        lsWindow->setExclusiveZone(-1);
        lsWindow->setScope(QStringLiteral("quasar"));
        if (window->width() > 0 && window->height() > 0) {
            lsWindow->setDesiredSize(QSize(window->width(), window->height()));
        }
        qCDebug(lcLauncher) << "layer-shell configured on LayerTop with desired size:" << window->size();
    }
#else
    Q_UNUSED(window);
#endif
}

} // namespace

class IconProvider : public QQuickImageProvider
{
public:
    IconProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {
        QPixmapCache::setCacheLimit(20480); // 20 MB cache
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        // Called from the render thread: all shared caches need locking.
        QMutexLocker lock(&m_mutex);

        int width = requestedSize.width() > 0 ? requestedSize.width() : 64;
        int height = requestedSize.height() > 0 ? requestedSize.height() : 64;
        if (size) {
            *size = QSize(width, height);
        }

        if (id.isEmpty()) {
            return QPixmap(width, height);
        }

        QString cacheKey = QString("%1_%2x%3").arg(id).arg(width).arg(height);
        QPixmap cachedPixmap;
        if (QPixmapCache::find(cacheKey, &cachedPixmap)) {
            return cachedPixmap;
        }

        if (m_negativeCache.contains(cacheKey)) {
            QIcon fallback = QIcon::fromTheme(QStringLiteral("application-x-executable"));
            return fallback.isNull() ? QPixmap(width, height) : fallback.pixmap(width, height);
        }

        // 1. Direct File Path Check (Absolute path or local file)
        if (id.startsWith(QLatin1Char('/')) || QFile::exists(id)) {
            QPixmap pix(id);
            if (!pix.isNull()) {
                QPixmap scaled = pix.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                QPixmapCache::insert(cacheKey, scaled);
                return scaled;
            }
        }

        // 2. Icon Theme Lookup
        QIcon icon = QIcon::fromTheme(id);
        if (!icon.isNull()) {
            QPixmap pix = icon.pixmap(width, height);
            if (!pix.isNull()) {
                QPixmapCache::insert(cacheKey, pix);
                return pix;
            }
        }

        // 3. Fallback search for JetBrains / Flatpak / custom app icons
        static const QStringList searchPaths = {
            QDir::homePath() + "/.local/share/icons",
            QDir::homePath() + "/.local/share/icons/hicolor/scalable/apps",
            QDir::homePath() + "/.local/share/flatpak/exports/share/icons",
            QDir::homePath() + "/.icons",
            "/var/lib/flatpak/exports/share/icons",
            "/usr/share/icons/hicolor/scalable/apps",
            "/usr/share/icons/hicolor/512x512/apps",
            "/usr/share/pixmaps"
        };

        QString cleanName = id;
        int lastSlash = cleanName.lastIndexOf(QLatin1Char('/'));
        if (lastSlash != -1) {
            cleanName = cleanName.mid(lastSlash + 1);
        }

        for (const QString &dir : searchPaths) {
            for (const QString &ext : { "", ".png", ".svg", ".xpm" }) {
                QString path = dir + "/" + cleanName + ext;
                if (QFile::exists(path)) {
                    QPixmap pix(path);
                    if (!pix.isNull()) {
                        QPixmap scaled = pix.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        QPixmapCache::insert(cacheKey, scaled);
                        return scaled;
                    }
                }
            }
        }

        m_negativeCache.insert(cacheKey);

        // 4. Default fallback icon
        QIcon fallback = QIcon::fromTheme(QStringLiteral("application-x-executable"));
        if (!fallback.isNull()) {
            return fallback.pixmap(width, height);
        }

        return QPixmap(width, height);
    }

private:
    QMutex m_mutex;
    QSet<QString> m_negativeCache;
};

int main(int argc, char *argv[])
{
    const QString dbusServiceName = "com.quasar.launcher";

    for (int i = 1; i < argc; ++i) {
        QString arg = QString(argv[i]);
        if (arg == "theme" || arg == "--theme" || arg == "theme-selector") {
            Platform::detachedStart("quasar-theme-selector", {});
            return 0;
        }
    }

    // Layer-shell must be enabled before the QGuiApplication instance exists,
    // and only makes sense in Wayland sessions; on X11 we fall back to a
    // frameless, centered, always-on-top window.
#ifdef QUASAR_HAVE_LAYERSHELL
    const bool useLayerShell = runningOnWayland();
    if (useLayerShell) {
        LayerShellQt::Shell::useLayerShell();
    }
#endif

    QGuiApplication app(argc, argv);
    app.setApplicationName("quasar");
    app.setOrganizationName("com.quasar");

    qCDebug(lcLauncher) << "platform:" << QGuiApplication::platformName()
                        << "compositor:" << Platform::compositorName(Platform::detectCompositor());

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(dbusServiceName)) {
        for (int i = 1; i < argc; ++i) {
            if (QString(argv[i]) == "--toggle" || QString(argv[i]) == "-t") {
                QDBusMessage msg = QDBusMessage::createMethodCall(dbusServiceName, "/Main", "com.quasar.launcher", "toggle");
                QDBusConnection::sessionBus().send(msg);
                QCoreApplication::processEvents();
                break;
            }
        }
        return 0;
    }

    QCommandLineParser parser;
    QCommandLineOption toggleOption("toggle", "Toggle launcher visibility");
    parser.addOption(toggleOption);
    parser.process(app);

    if (!QDBusConnection::sessionBus().registerService(dbusServiceName)) {
        qCWarning(lcLauncher) << "Failed to register D-Bus service:" << dbusServiceName;
    }

    LauncherController controller;
    if (!QDBusConnection::sessionBus().registerObject("/Main", "com.quasar.launcher", &controller, QDBusConnection::ExportAllSlots)) {
        qCWarning(lcLauncher) << "Failed to register D-Bus object /Main";
    }

    // Hide the preview canvas whenever the theme selector goes away,
    // whatever way it exits (close, crash, kill).
    auto *selectorWatcher = new QDBusServiceWatcher(QStringLiteral("com.quasar.themeSelector"),
        QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, &app);
    QObject::connect(selectorWatcher, &QDBusServiceWatcher::serviceUnregistered, &app,
                     [&controller](const QString &) {
        QMetaObject::invokeMethod(&controller, "endPreview");
    });

    AppIndexer *appIndexer = new AppIndexer(&app);
    WindowSwitcher *windowSwitcher = new WindowSwitcher(&app);
    ActionModel *actionModel = new ActionModel(&app);
    EmojiManager *emojiManager = new EmojiManager(&app);
    ClipboardManager *clipboardManager = new ClipboardManager(&app);

    FuzzyMatcher *fuzzyMatcher = new FuzzyMatcher(&app);
    fuzzyMatcher->setAppIndexerModel(appIndexer);
    fuzzyMatcher->setWindowModel(windowSwitcher->model());
    fuzzyMatcher->setActionModel(actionModel);
    fuzzyMatcher->setEmojiModel(emojiManager);
    fuzzyMatcher->setClipboardModel(clipboardManager);

    qmlRegisterSingletonInstance("com.quasar.launcher", 1, 0, "FrecencyRanker", FrecencyRanker::instance());
    qmlRegisterSingletonInstance("com.quasar.launcher", 1, 0, "AppIndexer", appIndexer);
    qmlRegisterSingletonInstance("com.quasar.launcher", 1, 0, "FuzzyMatcher", fuzzyMatcher);
    qmlRegisterSingletonInstance("com.quasar.launcher", 1, 0, "ThemeManager", ThemeManager::instance());
    qmlRegisterSingletonInstance("com.quasar.launcher", 1, 0, "ActionModel", actionModel);
    qmlRegisterSingletonInstance("com.quasar.launcher", 1, 0, "EmojiManager", emojiManager);
    qmlRegisterSingletonInstance("com.quasar.launcher", 1, 0, "ClipboardManager", clipboardManager);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("fuzzyMatcher", fuzzyMatcher);
    engine.rootContext()->setContextProperty("appIndexer", appIndexer);
    engine.rootContext()->setContextProperty("themeManager", ThemeManager::instance());
    engine.rootContext()->setContextProperty("frecencyRanker", FrecencyRanker::instance());
    engine.rootContext()->setContextProperty("windowSwitcher", windowSwitcher);
    engine.rootContext()->setContextProperty("emojiManager", emojiManager);
    engine.rootContext()->setContextProperty("clipboardManager", clipboardManager);
    engine.rootContext()->setContextProperty("FuzzyMatcher", fuzzyMatcher);
    engine.rootContext()->setContextProperty("AppIndexer", appIndexer);
    engine.rootContext()->setContextProperty("ThemeManager", ThemeManager::instance());
    engine.rootContext()->setContextProperty("FrecencyRanker", FrecencyRanker::instance());
    engine.rootContext()->setContextProperty("WindowSwitcher", windowSwitcher);
    engine.rootContext()->setContextProperty("EmojiManager", emojiManager);
    engine.rootContext()->setContextProperty("ClipboardManager", clipboardManager);
    engine.addImageProvider(QLatin1String("icon"), new IconProvider);

    QWindow *rootWindow = nullptr;

    auto showLauncher = [&rootWindow]() {
        if (!rootWindow) return;
        QScreen *targetScreen = Platform::focusedScreen();
        if (targetScreen) {
            rootWindow->setScreen(targetScreen);
#ifdef QUASAR_HAVE_LAYERSHELL
            LayerShellQt::Window *lsWindow = LayerShellQt::Window::get(rootWindow);
            if (lsWindow) {
                lsWindow->setScreen(targetScreen);
            }
#endif
        }
        configureLayerShell(rootWindow);
        rootWindow->show();
        rootWindow->requestActivate();
        rootWindow->raise();
        QMetaObject::invokeMethod(rootWindow, "onOpened");
    };

    auto doToggle = [&rootWindow, &showLauncher]() {
        if (!rootWindow) return;
        if (rootWindow->isVisible()) {
            rootWindow->hide();
        } else {
            showLauncher();
        }
    };

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [&rootWindow, &showLauncher, &parser, &toggleOption](QObject *obj, const QUrl &objUrl) {
        if (!obj) {
            qCWarning(lcLauncher) << "QML failed to load url:" << objUrl;
            QCoreApplication::exit(-1);
            return;
        }

        rootWindow = qobject_cast<QWindow *>(obj);
        if (rootWindow) {
            configureLayerShell(rootWindow);

            QObject::connect(rootWindow, &QWindow::activeChanged, rootWindow, [rootWindow]() {
                // While the theme selector overlay is up (QML sets
                // previewMode), the launcher stays visible for live preview.
                const bool previewMode = rootWindow->property("previewMode").toBool();
                if (!previewMode && !rootWindow->isActive() && rootWindow->isVisible()) {
                    rootWindow->hide();
                }
            });

            // If not run with --toggle, show the launcher immediately once loaded
            if (!parser.isSet(toggleOption)) {
                showLauncher();
            }
        }
    }, Qt::QueuedConnection);

    const QUrl url(QStringLiteral("qrc:/com/quasar/launcher/qml/Main.qml"));
    engine.load(url);

    QObject::connect(&controller, &LauncherController::toggleRequested, &app, doToggle);

    // The theme selector asks us to become its live-preview canvas.
    // Shown WITHOUT activation and WITHOUT keyboard grab — the selector
    // must keep focus, otherwise its search and arrow keys die.
    QObject::connect(&controller, &LauncherController::showPreviewRequested, &app,
                     [&rootWindow]() {
        if (!rootWindow) return;
#ifdef QUASAR_HAVE_LAYERSHELL
        LayerShellQt::Window *ls = LayerShellQt::Window::get(rootWindow);
        if (ls) {
            ls->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
            ls->setAnchors(LayerShellQt::Window::AnchorLeft);
            ls->setMargins(QMargins(50, 0, 0, 0));
        }
#endif
        if (!rootWindow->isVisible()) {
            rootWindow->show(); // no requestActivate, no onOpened
        }
        rootWindow->setProperty("previewMode", true);
    });

    QObject::connect(&controller, &LauncherController::endPreviewRequested, &app,
                     [&rootWindow]() {
        if (!rootWindow) return;
        rootWindow->setProperty("previewMode", false);
        rootWindow->hide();
        // Restore default layer-shell settings for normal launcher flow.
#ifdef QUASAR_HAVE_LAYERSHELL
        LayerShellQt::Window *ls = LayerShellQt::Window::get(rootWindow);
        if (ls) {
            ls->setAnchors(LayerShellQt::Window::Anchors(0));
            ls->setMargins(QMargins(0, 0, 0, 0));
        }
#endif
        configureLayerShell(rootWindow);
    });

    return app.exec();
}
