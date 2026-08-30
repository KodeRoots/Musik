// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2026 Musik Authors

#include "settings.h"

#include <KSharedConfig>

Settings::Settings(QObject *parent)
    : QObject(parent), m_config(KSharedConfig::openConfig()) {
  loadSettings();
}

int Settings::volume() const { return m_volume; }

void Settings::setVolume(int volume) {
  if (volume < 0)
    volume = 0;
  if (volume > 100)
    volume = 100;

  if (m_volume != volume) {
    m_volume = volume;
    Q_EMIT volumeChanged();
    saveSettings();
  }
}

bool Settings::muted() const { return m_muted; }

void Settings::setMuted(bool muted) {
  if (m_muted != muted) {
    m_muted = muted;
    Q_EMIT mutedChanged();
    saveSettings();
  }
}

bool Settings::showVolumeControls() const { return m_showVolumeControls; }

void Settings::setShowVolumeControls(bool show) {
  if (m_showVolumeControls != show) {
    m_showVolumeControls = show;
    Q_EMIT showVolumeControlsChanged();
    saveSettings();
  }
}

bool Settings::miniMode() const { return m_miniMode; }

void Settings::setMiniMode(bool miniMode) {
  if (m_miniMode != miniMode) {
    m_miniMode = miniMode;
    Q_EMIT miniModeChanged();
    saveSettings();
  }
}

bool Settings::noHeaderMode() const { return m_noHeaderMode; }

void Settings::setNoHeaderMode(bool noHeaderMode) {
  if (m_noHeaderMode != noHeaderMode) {
    m_noHeaderMode = noHeaderMode;
    Q_EMIT noHeaderModeChanged();
    saveSettings();
  }
}

QString Settings::lastFile() const { return m_lastFile; }

void Settings::setLastFile(const QString &lastFile) {
  if (m_lastFile != lastFile) {
    m_lastFile = lastFile;
    Q_EMIT lastFileChanged();
    saveSettings();
  }
}

qint64 Settings::lastPosition() const { return m_lastPosition; }

void Settings::setLastPosition(qint64 lastPosition) {
  if (m_lastPosition != lastPosition) {
    m_lastPosition = lastPosition;
    Q_EMIT lastPositionChanged();
    saveSettings();
  }
}

bool Settings::resumePlayback() const { return m_resumePlayback; }

void Settings::setResumePlayback(bool resume) {
  if (m_resumePlayback != resume) {
    m_resumePlayback = resume;
    Q_EMIT resumePlaybackChanged();
    saveSettings();
  }
}

bool Settings::restorePlaylist() const { return m_restorePlaylist; }

void Settings::setRestorePlaylist(bool restore) {
  if (m_restorePlaylist != restore) {
    m_restorePlaylist = restore;
    Q_EMIT restorePlaylistChanged();
    saveSettings();
  }
}

bool Settings::systemTrayEnabled() const { return m_systemTrayEnabled; }

void Settings::setSystemTrayEnabled(bool enabled) {
  if (m_systemTrayEnabled != enabled) {
    m_systemTrayEnabled = enabled;
    Q_EMIT systemTrayEnabledChanged();
    saveSettings();
  }
}

bool Settings::minimizeToTray() const { return m_minimizeToTray; }

void Settings::setMinimizeToTray(bool minimize) {
  if (m_minimizeToTray != minimize) {
    m_minimizeToTray = minimize;
    Q_EMIT minimizeToTrayChanged();
    saveSettings();
  }
}

bool Settings::startMinimized() const { return m_startMinimized; }

void Settings::setStartMinimized(bool minimized) {
  if (m_startMinimized != minimized) {
    m_startMinimized = minimized;
    Q_EMIT startMinimizedChanged();
    saveSettings();
  }
}

void Settings::loadSettings() {
  KConfigGroup audioGroup = m_config->group(QStringLiteral("Audio"));
  m_volume = audioGroup.readEntry(QStringLiteral("Volume"), 70);
  m_muted = audioGroup.readEntry(QStringLiteral("Muted"), false);
  m_showVolumeControls =
      audioGroup.readEntry(QStringLiteral("ShowVolumeControls"), false);
  m_miniMode = audioGroup.readEntry(QStringLiteral("MiniMode"), false);
  m_noHeaderMode = audioGroup.readEntry(QStringLiteral("NoHeaderMode"), false);
  m_resumePlayback =
      audioGroup.readEntry(QStringLiteral("ResumePlayback"), true);
  m_restorePlaylist =
      audioGroup.readEntry(QStringLiteral("RestorePlaylist"), true);

  KConfigGroup trayGroup = m_config->group(QStringLiteral("SystemTray"));
  m_systemTrayEnabled = trayGroup.readEntry(QStringLiteral("Enabled"), false);
  m_minimizeToTray =
      trayGroup.readEntry(QStringLiteral("MinimizeToTray"), false);
  m_startMinimized =
      trayGroup.readEntry(QStringLiteral("StartMinimized"), false);

  KConfigGroup playbackGroup = m_config->group(QStringLiteral("Playback"));
  m_lastFile = playbackGroup.readEntry(QStringLiteral("LastFile"), QString());
  m_lastPosition = playbackGroup.readEntry(QStringLiteral("LastPosition"), 0);
}

void Settings::saveSettings() {
  KConfigGroup audioGroup = m_config->group(QStringLiteral("Audio"));
  audioGroup.writeEntry(QStringLiteral("Volume"), m_volume);
  audioGroup.writeEntry(QStringLiteral("Muted"), m_muted);
  audioGroup.writeEntry(QStringLiteral("ShowVolumeControls"),
                        m_showVolumeControls);
  audioGroup.writeEntry(QStringLiteral("MiniMode"), m_miniMode);
  audioGroup.writeEntry(QStringLiteral("NoHeaderMode"), m_noHeaderMode);
  audioGroup.writeEntry(QStringLiteral("ResumePlayback"), m_resumePlayback);
  audioGroup.writeEntry(QStringLiteral("RestorePlaylist"), m_restorePlaylist);
  audioGroup.sync();

  KConfigGroup trayGroup = m_config->group(QStringLiteral("SystemTray"));
  trayGroup.writeEntry(QStringLiteral("Enabled"), m_systemTrayEnabled);
  trayGroup.writeEntry(QStringLiteral("MinimizeToTray"), m_minimizeToTray);
  trayGroup.writeEntry(QStringLiteral("StartMinimized"), m_startMinimized);
  trayGroup.sync();

  KConfigGroup playbackGroup = m_config->group(QStringLiteral("Playback"));
  playbackGroup.writeEntry(QStringLiteral("LastFile"), m_lastFile);
  playbackGroup.writeEntry(QStringLiteral("LastPosition"), m_lastPosition);
  playbackGroup.sync();
}
