#ifndef WINDOWSWITCHER_H
#define WINDOWSWITCHER_H

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class WindowSwitcher : public QObject {
    Q_OBJECT

public:
    explicit WindowSwitcher(QObject *parent = nullptr);

    Q_INVOKABLE QVariantList getOpenWindows();
    Q_INVOKABLE QVariantList getMatchingWindows(const QString &query);
    Q_INVOKABLE bool focusWindow(const QString &address);
};

#endif // WINDOWSWITCHER_H
