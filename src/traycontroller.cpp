// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2026 Musik Authors

#include "traycontroller.h"

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QSystemTrayIcon>

#include <KLocalizedString>

TrayController::TrayController(QObject *parent)
    : QObject(parent)
{
}

TrayController::~TrayController()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
    }
}

bool TrayController::enabled() const
{
    return m_enabled;
}

void TrayController::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }
    m_enabled = enabled;

    if (m_enabled && available()) {
        ensureTrayIcon();
        m_trayIcon->show();
    } else if (m_trayIcon) {
        m_trayIcon->hide();
    }

    Q_EMIT enabledChanged();
}

bool TrayController::available() const
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

QString TrayController::toolTip() const
{
    return m_toolTip;
}

void TrayController::setToolTip(const QString &toolTip)
{
    if (m_toolTip == toolTip) {
        return;
    }
    m_toolTip = toolTip;
    if (m_trayIcon) {
        m_trayIcon->setToolTip(m_toolTip);
    }
    Q_EMIT toolTipChanged();
}

bool TrayController::playing() const
{
    return m_playing;
}

void TrayController::setPlaying(bool playing)
{
    if (m_playing == playing) {
        return;
    }
    m_playing = playing;
    updatePlayPauseText();
    Q_EMIT playingChanged();
}

void TrayController::ensureTrayIcon()
{
    if (m_trayIcon) {
        return;
    }

    m_menu = new QMenu();

    m_playPauseAction = m_menu->addAction(QIcon::fromTheme(QStringLiteral("media-playback-start")), QString());
    connect(m_playPauseAction, &QAction::triggered, this, &TrayController::playPauseRequested);
    updatePlayPauseText();

    QAction *nextAction = m_menu->addAction(QIcon::fromTheme(QStringLiteral("media-skip-forward")), i18nc("@action:tray", "Next"));
    connect(nextAction, &QAction::triggered, this, &TrayController::nextRequested);

    QAction *previousAction = m_menu->addAction(QIcon::fromTheme(QStringLiteral("media-skip-backward")), i18nc("@action:tray", "Previous"));
    connect(previousAction, &QAction::triggered, this, &TrayController::previousRequested);

    m_menu->addSeparator();

    QAction *quitAction = m_menu->addAction(QIcon::fromTheme(QStringLiteral("application-exit")), i18nc("@action:tray", "Quit"));
    connect(quitAction, &QAction::triggered, this, &TrayController::quitRequested);

    m_trayIcon = new QSystemTrayIcon(QIcon::fromTheme(QStringLiteral("org.koderoots.musik")), this);
    m_trayIcon->setContextMenu(m_menu);
    m_trayIcon->setToolTip(m_toolTip);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            Q_EMIT showWindowRequested();
        }
    });
}

void TrayController::updatePlayPauseText()
{
    if (!m_playPauseAction) {
        return;
    }
    if (m_playing) {
        m_playPauseAction->setText(i18nc("@action:tray", "Pause"));
        m_playPauseAction->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-pause")));
    } else {
        m_playPauseAction->setText(i18nc("@action:tray", "Play"));
        m_playPauseAction->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));
    }
}
