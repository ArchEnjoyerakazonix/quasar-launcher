#pragma once

#include <QObject>

class LauncherController : public QObject {
    Q_OBJECT
public:
    explicit LauncherController(QObject *parent = nullptr) : QObject(parent) {}
    ~LauncherController() override = default;

public slots:
    Q_INVOKABLE void toggle() {
        emit toggleRequested();
    }

signals:
    void toggleRequested();
};
