#include <QGuiApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
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

#include "frecencyranker.h"
#include "appindexer.h"
#include "fuzzymatcher.h"
#include "thememanager.h"
#include "launchercontroller.h"

class IconProvider : public QQuickImageProvider
{
public:
    IconProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        int width = requestedSize.width() > 0 ? requestedSize.width() : 64;
        int height = requestedSize.height() > 0 ? requestedSize.height() : 64;
        if (size) {
            *size = QSize(width, height);
        }
        
        QIcon icon = QIcon::fromTheme(id);
        if (icon.isNull()) {
            return QPixmap(width, height);
        }
        return icon.pixmap(width, height);
    }
};

int main(int argc, char *argv[])
{
    const QString dbusServiceName = "com.nexus.launcher";

    {
        QCoreApplication checkApp(argc, argv);
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered(dbusServiceName)) {
            for (int i = 1; i < argc; ++i) {
                if (QString(argv[i]) == "--toggle" || QString(argv[i]) == "-t") {
                    QDBusMessage msg = QDBusMessage::createMethodCall(dbusServiceName, "/Main", "com.nexus.launcher", "toggle");
                    QDBusConnection::sessionBus().send(msg);
                    QCoreApplication::processEvents();
                    break;
                }
            }
            return 0;
        }
    }

    QGuiApplication app(argc, argv);
    app.setApplicationName("nexus-launcher");
    app.setOrganizationName("com.nexus");

    QCommandLineParser parser;
    QCommandLineOption toggleOption("toggle", "Toggle launcher visibility");
    parser.addOption(toggleOption);
    parser.process(app);

    QDBusConnection::sessionBus().registerService(dbusServiceName);

    LauncherController controller;
    QDBusConnection::sessionBus().registerObject("/Main", "com.nexus.launcher", &controller, QDBusConnection::ExportAllSlots);

    AppIndexer *appIndexer = new AppIndexer(&app);
    FuzzyMatcher *fuzzyMatcher = new FuzzyMatcher(&app);
    fuzzyMatcher->setSourceModel(appIndexer);

    qmlRegisterSingletonInstance("Nexus", 1, 0, "FrecencyRanker", FrecencyRanker::instance());
    qmlRegisterSingletonInstance("Nexus", 1, 0, "AppIndexer", appIndexer);
    qmlRegisterSingletonInstance("Nexus", 1, 0, "FuzzyMatcher", fuzzyMatcher);
    qmlRegisterSingletonInstance("Nexus", 1, 0, "ThemeManager", ThemeManager::instance());

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("icon"), new IconProvider);

    QWindow *rootWindow = nullptr;

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [&rootWindow](QObject *obj, const QUrl &objUrl) {
        if (!obj)
            QCoreApplication::exit(-1);
            
        rootWindow = qobject_cast<QWindow *>(obj);
        if (rootWindow) {
            LayerShellQt::Window *lsWindow = LayerShellQt::Window::get(rootWindow);
            if (lsWindow) {
                lsWindow->setLayer(LayerShellQt::Window::LayerOverlay);
                lsWindow->setAnchors(LayerShellQt::Window::Anchors()); // Centered, no edge anchors
                lsWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityExclusive);
                lsWindow->setExclusiveZone(-1);
                lsWindow->setScope(QStringLiteral("nexus-launcher"));
            }
        }
    }, Qt::QueuedConnection);

    const QUrl url(QStringLiteral("qrc:/com/nexus/launcher/qml/Main.qml"));
    engine.load(url);

    auto doToggle = [&rootWindow]() {
        if (!rootWindow) return;
        if (rootWindow->isVisible()) {
            rootWindow->hide();
        } else {
            rootWindow->show();
            rootWindow->requestActivate();
            QMetaObject::invokeMethod(rootWindow, "onOpened");
        }
    };

    QObject::connect(&controller, &LauncherController::toggleRequested, &app, doToggle);

    // Initial show if run directly without --toggle
    if (!parser.isSet(toggleOption)) {
        QTimer::singleShot(100, doToggle);
    }

    return app.exec();
}
