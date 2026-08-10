#include "windowswitcher.h"
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

WindowSwitcher::WindowSwitcher(QObject *parent)
    : QObject(parent)
{
}

QVariantList WindowSwitcher::getOpenWindows()
{
    QVariantList windows;

    QProcess process;
    process.start("hyprctl", QStringList() << "clients" << "-j");
    if (!process.waitForFinished(1000)) {
        qWarning() << "WindowSwitcher: hyprctl clients timed out";
        return windows;
    }

    QByteArray output = process.readAllStandardOutput();
    QJsonDocument doc = QJsonDocument::fromJson(output);
    if (!doc.isArray()) {
        return windows;
    }

    QJsonArray array = doc.array();
    for (const QJsonValue &val : array) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();

        QString title = obj["title"].toString();
        QString cls = obj["class"].toString();
        QString address = obj["address"].toString();
        bool mapped = obj["mapped"].toBool(true);

        if (!mapped || (title.isEmpty() && cls.isEmpty())) {
            continue;
        }

        QVariantMap win;
        win["title"] = title.isEmpty() ? cls : title;
        win["class"] = cls;
        win["address"] = address;
        win["workspace"] = obj["workspace"].toObject()["name"].toString();
        win["iconName"] = cls.toLower();

        windows.append(win);
    }

    return windows;
}

bool WindowSwitcher::focusWindow(const QString &address)
{
    if (address.isEmpty()) return false;
    QProcess::startDetached("hyprctl", QStringList() << "dispatch" << "focuswindow" << QString("address:%1").arg(address));
    return true;
}
