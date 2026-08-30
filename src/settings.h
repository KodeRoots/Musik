// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2026 Musik Authors

#ifndef SETTINGS_H
#define SETTINGS_H

#include <KConfigGroup>
#include <KSharedConfig>
#include <QObject>
#include <QQmlEngine>

class Settings : public QObject {
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
  Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY mutedChanged)
  Q_PROPERTY(bool showVolumeControls READ showVolumeControls WRITE
                 setShowVolumeControls NOTIFY showVolumeControlsChanged)
  Q_PROPERTY(
      bool miniMode READ miniMode WRITE setMiniMode NOTIFY miniModeChanged)
  Q_PROPERTY(bool noHeaderMode READ noHeaderMode WRITE setNoHeaderMode NOTIFY
                 noHeaderModeChanged)
  Q_PROPERTY(
      QString lastFile READ lastFile WRITE setLastFile NOTIFY lastFileChanged)
  Q_PROPERTY(qint64 lastPosition READ lastPosition WRITE setLastPosition NOTIFY
                 lastPositionChanged)
  Q_PROPERTY(bool resumePlayback READ resumePlayback WRITE setResumePlayback
                 NOTIFY resumePlaybackChanged)
  Q_PROPERTY(bool restorePlaylist READ restorePlaylist WRITE setRestorePlaylist
                 NOTIFY restorePlaylistChanged)
  Q_PROPERTY(bool systemTrayEnabled READ systemTrayEnabled WRITE
                 setSystemTrayEnabled NOTIFY systemTrayEnabledChanged)
  Q_PROPERTY(bool minimizeToTray READ minimizeToTray WRITE setMinimizeToTray
                 NOTIFY minimizeToTrayChanged)
  Q_PROPERTY(bool startMinimized READ startMinimized WRITE setStartMinimized
                 NOTIFY startMinimizedChanged)

public:
  explicit Settings(QObject *parent = nullptr);

  int volume() const;
  void setVolume(int volume);

  bool muted() const;
  void setMuted(bool muted);

  bool showVolumeControls() const;
  void setShowVolumeControls(bool show);

  bool miniMode() const;
  void setMiniMode(bool miniMode);

  bool noHeaderMode() const;
  void setNoHeaderMode(bool noHeaderMode);

  QString lastFile() const;
  void setLastFile(const QString &lastFile);

  qint64 lastPosition() const;
  void setLastPosition(qint64 lastPosition);

  bool resumePlayback() const;
  void setResumePlayback(bool resume);

  bool restorePlaylist() const;
  void setRestorePlaylist(bool restore);

  bool systemTrayEnabled() const;
  void setSystemTrayEnabled(bool enabled);

  bool minimizeToTray() const;
  void setMinimizeToTray(bool minimize);

  bool startMinimized() const;
  void setStartMinimized(bool minimized);

  static Settings *create(QQmlEngine *, QJSEngine *) { return new Settings; }

Q_SIGNALS:
  void volumeChanged();
  void mutedChanged();
  void showVolumeControlsChanged();
  void miniModeChanged();
  void noHeaderModeChanged();
  void lastFileChanged();
  void lastPositionChanged();
  void resumePlaybackChanged();
  void restorePlaylistChanged();
  void systemTrayEnabledChanged();
  void minimizeToTrayChanged();
  void startMinimizedChanged();

private:
  void loadSettings();
  void saveSettings();

  KSharedConfig::Ptr m_config;
  int m_volume = 70;
  bool m_muted = false;
  bool m_showVolumeControls = false;
  bool m_miniMode = false;
  bool m_noHeaderMode = false;
  QString m_lastFile;
  qint64 m_lastPosition = 0;
  bool m_resumePlayback = true;
  bool m_restorePlaylist = true;
  bool m_systemTrayEnabled = false;
  bool m_minimizeToTray = false;
  bool m_startMinimized = false;
};

#endif // SETTINGS_H
