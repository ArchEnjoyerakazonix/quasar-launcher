#include <QGuiApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusConnectionInterface>
#include <QCommandLineParser>
#include <QWindow>
#include <LayerShellQt/window.h>
#include <LayerShellQt/shell.h>
#include <QIcon>
#include <QQuickImageProvider>
#include <QPixmap>
#include <QFile>
#include <QDir>
#include <QDebug>

#include "frecencyranker.h"
#include "appindexer.h"
#include "fuzzymatcher.h"
#include "thememanager.h"
#include "launchercontroller.h"

#include <QPixmapCache>
#include <QSet>
#include <QScreen>
#include <QCursor>

class IconProvider : public QQuickImageProvider
{
public:
    IconProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {
        QPixmapCache::setCacheLimit(20480); // 20 MB cache
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override
    {
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

        static QSet<QString> negativeCache;
        if (negativeCache.contains(cacheKey)) {
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

        negativeCache.insert(cacheKey);

        // 4. Default fallback icon
        QIcon fallback = QIcon::fromTheme(QStringLiteral("application-x-executable"));
        if (!fallback.isNull()) {
            return fallback.pixmap(width, height);
        }

        return QPixmap(width, height);
    }
};

int main(int argc, char *argv[])
{
    LayerShellQt::Shell::useLayerShell();
    const QString dbusServiceName = "com.quasar.launcher";

    QGuiApplication app(argc, argv);
    app.setApplicationName("quasar");
    app.setOrganizationName("com.quasar");

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
        qWarning() << "Failed to register D-Bus service:" << dbusServiceName;
    }

    LauncherController controller;
    if (!QDBusConnection::sessionBus().registerObject("/Main", "com.quasar.launcher", &controller, QDBusConnection::ExportAllSlots)) {
        qWarning() << "Failed to register D-Bus object /Main";
    }

    AppIndexer *appIndexer = new AppIndexer(&app);
    FuzzyMatcher *fuzzyMatcher = new FuzzyMatcher(&app);
    fuzzyMatcher->setSourceModel(appIndexer);

    qmlRegisterSingletonInstance("com.quasar.launcher", 1, 0, "FrecencyRanker", FrecencyRanker::instance());
    qmlRegisterSingletonInstance("com.quasar.launcher", 1, 0, "AppIndexer", appIndexer);
    qmlRegisterSingletonInstance("com.quasar.launcher", 1, 0, "FuzzyMatcher", fuzzyMatcher);
    qmlRegisterSingletonInstance("com.quasar.launcher", 1, 0, "ThemeManager", ThemeManager::instance());

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("fuzzyMatcher", fuzzyMatcher);
    engine.rootContext()->setContextProperty("appIndexer", appIndexer);
    engine.rootContext()->setContextProperty("themeManager", ThemeManager::instance());
    engine.rootContext()->setContextProperty("frecencyRanker", FrecencyRanker::instance());
    engine.rootContext()->setContextProperty("FuzzyMatcher", fuzzyMatcher);
    engine.rootContext()->setContextProperty("AppIndexer", appIndexer);
    engine.rootContext()->setContextProperty("ThemeManager", ThemeManager::instance());
    engine.rootContext()->setContextProperty("FrecencyRanker", FrecencyRanker::instance());
    engine.addImageProvider(QLatin1String("icon"), new IconProvider);

    QWindow *rootWindow = nullptr;

    auto doToggle = [&rootWindow]() {
        if (!rootWindow) return;
        if (rootWindow->isVisible()) {
            rootWindow->hide();
        } else {
            QScreen *cursorScreen = QGuiApplication::screenAt(QCursor::pos());
            if (cursorScreen) {
                rootWindow->setScreen(cursorScreen);
            }
            rootWindow->show();
            rootWindow->requestActivate();
            QMetaObject::invokeMethod(rootWindow, "onOpened");
        }
    };

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [&rootWindow, &parser, &toggleOption](QObject *obj, const QUrl &objUrl) {
        qDebug() << "QML objectCreated: obj=" << obj << "url=" << objUrl;
        if (!obj) {
            qDebug() << "QML failed to load url:" << objUrl;
            QCoreApplication::exit(-1);
            return;
        }
            
        rootWindow = qobject_cast<QWindow *>(obj);
        if (rootWindow) {
            qDebug() << "rootWindow is successfully casted. Setting up LayerShellQt.";
            LayerShellQt::Window *lsWindow = LayerShellQt::Window::get(rootWindow);
            if (lsWindow) {
                lsWindow->setLayer(LayerShellQt::Window::LayerOverlay);
                lsWindow->setAnchors(LayerShellQt::Window::Anchors(
                                     LayerShellQt::Window::AnchorLeft | 
                                     LayerShellQt::Window::AnchorRight | 
                                     LayerShellQt::Window::AnchorTop | 
                                     LayerShellQt::Window::AnchorBottom));
                lsWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityExclusive);
                lsWindow->setExclusiveZone(-1);
                lsWindow->setScope(QStringLiteral("quasar"));
                qDebug() << "LayerShellQt successfully configured.";
            }

            // If not run with --toggle, show the launcher immediately once loaded
            if (!parser.isSet(toggleOption)) {
                qDebug() << "Showing window now.";
                rootWindow->show();
                rootWindow->requestActivate();
                QMetaObject::invokeMethod(rootWindow, "onOpened");
            }
        }
    }, Qt::QueuedConnection);

    const QUrl url(QStringLiteral("qrc:/com/quasar/launcher/qml/Main.qml"));
    engine.load(url);

    QObject::connect(&controller, &LauncherController::toggleRequested, &app, doToggle);

    return app.exec();
}
