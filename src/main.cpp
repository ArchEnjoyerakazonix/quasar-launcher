#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QCommandLineParser>
#include <QWindow>
#include <LayerShellQt/window.h>
#include <LayerShellQt/shell.h>
#include <QTimer>

#include "frecencyranker.h"
#include "appindexer.h"
#include "fuzzymatcher.h"
#include "thememanager.h"

#include <QIcon>
#include <QQuickImageProvider>
#include <QPixmap>

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
            return QPixmap(width, height); // Return empty pixmap
        }
        return icon.pixmap(width, height);
    }
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("nexus-launcher");
    app.setOrganizationName("com.nexus");

    QCommandLineParser parser;
    QCommandLineOption toggleOption("toggle", "Toggle launcher visibility");
    parser.addOption(toggleOption);
    parser.process(app);

    const QString dbusServiceName = "com.nexus.launcher";

    // Handle single instance and toggle
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(dbusServiceName)) {
        if (parser.isSet(toggleOption)) {
            QDBusInterface iface(dbusServiceName, "/Main", "com.nexus.launcher");
            iface.call("toggleVisibility");
        }
        return 0;
    }

    QDBusConnection::sessionBus().registerService(dbusServiceName);

    AppIndexer *appIndexer = new AppIndexer(&app);
    FuzzyMatcher *fuzzyMatcher = new FuzzyMatcher(&app);
    fuzzyMatcher->setSourceModel(appIndexer);

    qmlRegisterSingletonInstance("Nexus", 1, 0, "FrecencyRanker", FrecencyRanker::instance());
    qmlRegisterSingletonInstance("Nexus", 1, 0, "AppIndexer", appIndexer);
    qmlRegisterSingletonInstance("Nexus", 1, 0, "FuzzyMatcher", fuzzyMatcher);
    qmlRegisterSingletonInstance("Nexus", 1, 0, "ThemeManager", ThemeManager::instance());

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("icon"), new IconProvider);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [](QObject *obj, const QUrl &objUrl) {
        if (!obj)
            QCoreApplication::exit(-1);
            
        // Configure LayerShellQt on the root window
        QWindow *window = qobject_cast<QWindow *>(obj);
        if (window) {
            LayerShellQt::Window *lsWindow = LayerShellQt::Window::get(window);
            if (lsWindow) {
                lsWindow->setLayer(LayerShellQt::Window::LayerOverlay);
                lsWindow->setAnchors(LayerShellQt::Window::Anchors(
                                     LayerShellQt::Window::AnchorLeft | 
                                     LayerShellQt::Window::AnchorRight | 
                                     LayerShellQt::Window::AnchorTop | 
                                     LayerShellQt::Window::AnchorBottom));
                lsWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityExclusive);
                lsWindow->setExclusiveZone(-1);
                lsWindow->setScope(QStringLiteral("nexus-launcher"));
            }
        }
    }, Qt::QueuedConnection);

    const QUrl url(QStringLiteral("qrc:/com/nexus/launcher/qml/Main.qml"));
    engine.load(url);
    
    QObject *rootObject = engine.rootObjects().isEmpty() ? nullptr : engine.rootObjects().first();
    if (rootObject) {
        QDBusConnection::sessionBus().registerObject("/Main", rootObject, QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableInvokables);
        
        if (parser.isSet(toggleOption)) {
            QMetaObject::invokeMethod(rootObject, "toggleVisibility");
        }
    }

    return app.exec();
}
