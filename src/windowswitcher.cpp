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

QVariantList WindowSwitcher::getMatchingWindows(const QString &query)
{
    QVariantList all = getOpenWindows();
    if (query.trimmed().isEmpty()) {
        return all;
    }

    QString cleanQuery = query.trimmed();
    if (cleanQuery.startsWith("w:")) {
        cleanQuery = cleanQuery.mid(2).trimmed();
    } else if (cleanQuery.startsWith("window:")) {
        cleanQuery = cleanQuery.mid(7).trimmed();
    }

    if (cleanQuery.isEmpty()) {
        return all;
    }

    QVariantList filtered;
    for (const QVariant &var : all) {
        QVariantMap map = var.toMap();
        QString title = map["title"].toString();
        QString cls = map["class"].toString();
        QString workspace = map["workspace"].toString();

        if (title.contains(cleanQuery, Qt::CaseInsensitive) ||
            cls.contains(cleanQuery, Qt::CaseInsensitive) ||
            workspace.contains(cleanQuery, Qt::CaseInsensitive)) {
            filtered.append(map);
        }
    }
    return filtered;
}

bool WindowSwitcher::focusWindow(const QString &address)
{
    if (address.isEmpty()) return false;
    QProcess::startDetached("hyprctl", QStringList() << "dispatch" << "focuswindow" << QString("address:%1").arg(address));
    return true;
}
