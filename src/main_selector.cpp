#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QQuickImageProvider>
#include <QPixmap>
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
    QGuiApplication app(argc, argv);
    app.setApplicationName("quasar-theme-selector");
    app.setDesktopFileName("quasar-theme-selector");
    app.setOrganizationName("com.quasar");

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("icon"), new IconProvider);
    
    qmlRegisterSingletonInstance("Quasar", 1, 0, "ThemeManager", ThemeManager::instance());

    const QUrl url(QStringLiteral("qrc:/com/quasar/themeselector/theme_selector/SelectorMain.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
