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
    // Called by the theme selector so the launcher window becomes the
    // live-preview canvas and ignores focus loss while the selector is up.
    Q_INVOKABLE void showPreview() {
        emit showPreviewRequested();
    }
    // The selector closed — hide the preview canvas and restore normal
    // auto-hide behaviour.
    Q_INVOKABLE void endPreview() {
        emit endPreviewRequested();
    }

signals:
    void toggleRequested();
    void showPreviewRequested();
    void endPreviewRequested();
};
