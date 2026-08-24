#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QQuickImageProvider>
#include <QPixmap>
#include <QWindow>
#include <QScreen>
#include <QCursor>

#ifdef QUASAR_HAVE_LAYERSHELL
#include <LayerShellQt/window.h>
#include <LayerShellQt/shell.h>
#endif

#include "thememanager.h"

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
    qunsetenv("QT_WAYLAND_SHELL_INTEGRATION");

    QGuiApplication app(argc, argv);
    app.setApplicationName("quasar-theme-selector");
    app.setDesktopFileName("quasar-theme-selector");
    app.setOrganizationName("com.quasar");

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("icon"), new IconProvider);

    qmlRegisterSingletonInstance("Quasar", 1, 0, "ThemeManager", ThemeManager::instance());

    const QUrl url(QStringLiteral("qrc:/com/quasar/themeselector/qml/theme_selector/SelectorMain.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
            return;
        }

        QWindow *window = qobject_cast<QWindow*>(obj);
        if (window) {
#ifdef QUASAR_HAVE_LAYERSHELL
            if (QGuiApplication::platformName() != QLatin1String("xcb")) {
                // Wayland: full-screen transparent overlay.
                // The QML handles the split layout internally:
                // left half = embedded Quasar preview, right half = theme list.
                LayerShellQt::Window *ls = LayerShellQt::Window::get(window);
                if (ls) {
                    ls->setLayer(LayerShellQt::Window::LayerOverlay);
                    ls->setAnchors(LayerShellQt::Window::Anchors(
                        LayerShellQt::Window::AnchorTop | LayerShellQt::Window::AnchorBottom |
                        LayerShellQt::Window::AnchorLeft | LayerShellQt::Window::AnchorRight));
                    ls->setExclusiveZone(-1);
                    ls->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
                    ls->setScope(QStringLiteral("quasar-theme-selector"));
                }
                QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
                if (screen) {
                    window->setScreen(screen);
                }
            } else
#endif
            {
                // X11 fallback: size and position the overlay explicitly.
                QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
                if (!screen) {
                    screen = QGuiApplication::primaryScreen();
                }
                if (screen) {
                    window->setScreen(screen);
                    window->setGeometry(screen->geometry());
                }
            }
            window->show();
            window->requestActivate();
        }
    }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
