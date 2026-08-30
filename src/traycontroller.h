// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2026 Musik Authors

#ifndef TRAYCONTROLLER_H
#define TRAYCONTROLLER_H

#include <QObject>
#include <QQmlEngine>

class QSystemTrayIcon;
class QMenu;
class QAction;

/**
 * @brief System tray icon controller
 *
 * TrayController wraps QSystemTrayIcon and exposes playback actions
 * (play/pause, next, previous, quit) plus window restore to QML.
 */
class TrayController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip NOTIFY toolTipChanged)
    Q_PROPERTY(bool playing READ playing WRITE setPlaying NOTIFY playingChanged)

public:
    explicit TrayController(QObject *parent = nullptr);
    ~TrayController() override;

    bool enabled() const;
    void setEnabled(bool enabled);

    bool available() const;

    QString toolTip() const;
    void setToolTip(const QString &toolTip);

    bool playing() const;
    void setPlaying(bool playing);

Q_SIGNALS:
    void enabledChanged();
    void toolTipChanged();
    void playingChanged();

    void showWindowRequested();
    void playPauseRequested();
    void nextRequested();
    void previousRequested();
    void quitRequested();

private:
    void ensureTrayIcon();
    void updatePlayPauseText();

    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_menu = nullptr;
    QAction *m_playPauseAction = nullptr;
    bool m_enabled = false;
    QString m_toolTip;
    bool m_playing = false;
};

#endif // TRAYCONTROLLER_H
