// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2026 Musik Authors

#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QUrl>

/**
 * @brief Represents a single track in the playlist
 */
struct Track {
    QUrl url;
    QString title;
    QString artist;
    QString album;
    QString albumArtPath;
    qint64 duration = 0;
};

/**
 * @brief Model for managing the audio playlist
 *
 * PlaylistModel provides a QAbstractListModel-based interface for managing
 * a list of audio tracks. It handles metadata extraction, track navigation,
 * and supports shuffle and repeat modes.
 */
class PlaylistModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QUrl currentUrl READ currentUrl NOTIFY currentIndexChanged)

public:
    enum Roles {
        UrlRole = Qt::UserRole + 1,
        TitleRole,
        ArtistRole,
        AlbumRole,
        AlbumArtRole,
        DurationRole
    };

    explicit PlaylistModel(QObject *parent = nullptr);
    ~PlaylistModel() override;

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Properties
    int count() const;
    int currentIndex() const;
    void setCurrentIndex(int index);
    QUrl currentUrl() const;

    /**
     * @brief Add multiple tracks to the playlist
     * @param urls List of file URLs to add
     */
    Q_INVOKABLE void addTracks(const QList<QUrl> &urls);

    /**
     * @brief Remove a track from the playlist
     * @param index Index of the track to remove
     */
    Q_INVOKABLE void removeTrack(int index);

    /**
     * @brief Clear all tracks from the playlist
     */
    Q_INVOKABLE void clear();

    /**
     * @brief Get the URL of a track at a specific index
     * @param index Index of the track
     * @return URL of the track, or empty URL if index is invalid
     */
    Q_INVOKABLE QUrl urlAt(int index) const;

    /**
     * @brief Calculate the next track index based on playback mode
     * @param shuffle Whether shuffle mode is enabled
     * @param repeatMode 0=Off, 1=One, 2=All
     * @return Next track index, or -1 if at end (repeat off)
     */
    Q_INVOKABLE int nextIndex(bool shuffle, int repeatMode) const;

    /**
     * @brief Calculate the previous track index
     * @return Previous track index, or current index if at start
     */
    Q_INVOKABLE int previousIndex() const;

    /**
     * @brief Save the playlist to a file (.m3u, .m3u8 or .pls by extension)
     * @param fileUrl Local file URL of the playlist file
     * @return true on success
     */
    Q_INVOKABLE bool saveToFile(const QUrl &fileUrl);

    /**
     * @brief Load tracks from a playlist file (.m3u, .m3u8 or .pls)
     * @param fileUrl Local file URL of the playlist file
     * @return true if at least one track was loaded
     */
    Q_INVOKABLE bool loadFromFile(const QUrl &fileUrl);

    /**
     * @brief Save the playlist to the default auto-save location
     */
    Q_INVOKABLE void saveToDefault();

    /**
     * @brief Load the playlist from the default auto-save location
     */
    Q_INVOKABLE void loadFromDefault();

Q_SIGNALS:
    void countChanged();
    void currentIndexChanged();
    void trackAdded(int index);
    void errorOccurred(const QString &message);

private:
    Track extractTrackMetadata(const QUrl &url);
    void cleanupTempFiles();
    bool isAudioFile(const QString &path) const;
    bool writeM3U(const QString &path);
    bool writePLS(const QString &path);
    QList<QUrl> parsePlaylist(const QString &path) const;
    static QString defaultPlaylistPath();

    QList<Track> m_tracks;
    int m_currentIndex = -1;
    QStringList m_tempArtPaths;
};

#endif // PLAYLISTMODEL_H
