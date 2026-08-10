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
#include <QDebug>

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

    QDBusConnection::sessionBus().registerService(dbusServiceName);

    LauncherController controller;
    QDBusConnection::sessionBus().registerObject("/Main", "com.quasar.launcher", &controller, QDBusConnection::ExportAllSlots);

    AppIndexer *appIndexer = new AppIndexer(&app);
    FuzzyMatcher *fuzzyMatcher = new FuzzyMatcher(&app);
    fuzzyMatcher->setSourceModel(appIndexer);

    qmlRegisterSingletonInstance("com.quasar.launcher", 1, 0, "FrecencyRanker", FrecencyRanker::instance());
    qmlRegisterSingletonInstance("com.quasar.launcher", 1, 0, "AppIndexer", appIndexer);
    qmlRegisterSingletonInstance("com.quasar.launcher", 1, 0, "FuzzyMatcher", fuzzyMatcher);
    qmlRegisterSingletonInstance("com.quasar.launcher", 1, 0, "ThemeManager", ThemeManager::instance());

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("icon"), new IconProvider);

    QWindow *rootWindow = nullptr;

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
