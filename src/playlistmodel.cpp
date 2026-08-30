// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2026 Musik Authors

#include "playlistmodel.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextStream>

#include <KLocalizedString>

#include <fileref.h>
#include <tag.h>
#include <tpropertymap.h>

PlaylistModel::PlaylistModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

PlaylistModel::~PlaylistModel()
{
    cleanupTempFiles();
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_tracks.size();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tracks.size()) {
        return QVariant();
    }

    const Track &track = m_tracks.at(index.row());

    switch (role) {
    case UrlRole:
        return track.url;
    case TitleRole:
        return track.title;
    case ArtistRole:
        return track.artist;
    case AlbumRole:
        return track.album;
    case AlbumArtRole:
        return track.albumArtPath;
    case DurationRole:
        return track.duration;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> PlaylistModel::roleNames() const
{
    return {
        {UrlRole, "url"},
        {TitleRole, "title"},
        {ArtistRole, "artist"},
        {AlbumRole, "album"},
        {AlbumArtRole, "albumArt"},
        {DurationRole, "duration"}
    };
}

int PlaylistModel::count() const
{
    return m_tracks.size();
}

int PlaylistModel::currentIndex() const
{
    return m_currentIndex;
}

void PlaylistModel::setCurrentIndex(int index)
{
    if (index < -1) {
        index = -1;
    }
    if (index >= m_tracks.size()) {
        index = m_tracks.size() - 1;
    }
    if (m_currentIndex != index) {
        m_currentIndex = index;
        Q_EMIT currentIndexChanged();
    }
}

QUrl PlaylistModel::currentUrl() const
{
    if (m_currentIndex >= 0 && m_currentIndex < m_tracks.size()) {
        return m_tracks.at(m_currentIndex).url;
    }
    return QUrl();
}

void PlaylistModel::addTracks(const QList<QUrl> &urls)
{
    if (urls.isEmpty()) {
        return;
    }

    QList<Track> newTracks;
    for (const QUrl &url : urls) {
        if (!url.isLocalFile()) {
            continue;
        }

        QString path = url.toLocalFile();
        if (!QFile::exists(path)) {
            Q_EMIT errorOccurred(i18n("File not found: %1", path));
            continue;
        }

        if (!isAudioFile(path)) {
            continue;
        }

        Track track = extractTrackMetadata(url);
        newTracks.append(track);
    }

    if (newTracks.isEmpty()) {
        return;
    }

    int first = m_tracks.size();
    int last = first + newTracks.size() - 1;

    beginInsertRows(QModelIndex(), first, last);
    m_tracks.append(newTracks);
    endInsertRows();

    Q_EMIT countChanged();

    // Auto-select first track if playlist was empty
    if (m_currentIndex < 0 && !m_tracks.isEmpty()) {
        setCurrentIndex(0);
    }

    // Emit trackAdded for each new track
    for (int i = first; i <= last; ++i) {
        Q_EMIT trackAdded(i);
    }
}

void PlaylistModel::removeTrack(int index)
{
    if (index < 0 || index >= m_tracks.size()) {
        return;
    }

    // Clean up temp album art file for this track
    const Track &track = m_tracks.at(index);
    if (!track.albumArtPath.isEmpty()) {
        QString tempPath = track.albumArtPath;
        if (tempPath.startsWith(QStringLiteral("file://"))) {
            tempPath = tempPath.mid(7);
        }
        if (QFile::exists(tempPath)) {
            QFile::remove(tempPath);
            m_tempArtPaths.removeAll(tempPath);
        }
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_tracks.removeAt(index);
    endRemoveRows();

    // Adjust current index if needed
    if (m_tracks.isEmpty()) {
        setCurrentIndex(-1);
    } else if (index < m_currentIndex) {
        // Track before current was removed, decrement
        m_currentIndex--;
        Q_EMIT currentIndexChanged();
    } else if (index == m_currentIndex) {
        // Current track was removed
        if (m_currentIndex >= m_tracks.size()) {
            setCurrentIndex(m_tracks.size() - 1);
        } else {
            // Stay at same index (which is now the next track)
            Q_EMIT currentIndexChanged();
        }
    }

    Q_EMIT countChanged();
}

void PlaylistModel::clear()
{
    if (m_tracks.isEmpty()) {
        return;
    }

    beginResetModel();
    cleanupTempFiles();
    m_tracks.clear();
    m_currentIndex = -1;
    endResetModel();

    Q_EMIT countChanged();
    Q_EMIT currentIndexChanged();
}

QUrl PlaylistModel::urlAt(int index) const
{
    if (index < 0 || index >= m_tracks.size()) {
        return QUrl();
    }
    return m_tracks.at(index).url;
}

int PlaylistModel::nextIndex(bool shuffle, int repeatMode) const
{
    if (m_tracks.isEmpty()) {
        return -1;
    }

    // RepeatMode: 0 = Off, 1 = One, 2 = All
    if (repeatMode == 1) {
        // Repeat current track
        return m_currentIndex;
    }

    if (shuffle) {
        // Random index excluding current (if more than one track)
        if (m_tracks.size() == 1) {
            return 0;
        }
        int next;
        do {
            next = QRandomGenerator::global()->bounded(m_tracks.size());
        } while (next == m_currentIndex);
        return next;
    }

    // Sequential next
    int next = m_currentIndex + 1;
    if (next >= m_tracks.size()) {
        if (repeatMode == 2) {
            // Repeat All: wrap to beginning
            return 0;
        }
        // Repeat Off: end of playlist
        return -1;
    }
    return next;
}

int PlaylistModel::previousIndex() const
{
    if (m_tracks.isEmpty() || m_currentIndex <= 0) {
        return m_currentIndex;
    }
    return m_currentIndex - 1;
}

QString PlaylistModel::defaultPlaylistPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/playlist.m3u8");
}

bool PlaylistModel::saveToFile(const QUrl &fileUrl)
{
    const QString path = fileUrl.toLocalFile();
    if (path.isEmpty()) {
        Q_EMIT errorOccurred(i18n("Invalid playlist file path"));
        return false;
    }

    if (path.endsWith(QLatin1String(".pls"), Qt::CaseInsensitive)) {
        return writePLS(path);
    }
    return writeM3U(path);
}

bool PlaylistModel::loadFromFile(const QUrl &fileUrl)
{
    const QString path = fileUrl.toLocalFile();
    if (!QFile::exists(path)) {
        Q_EMIT errorOccurred(i18n("Playlist file not found: %1", path));
        return false;
    }

    const QList<QUrl> urls = parsePlaylist(path);
    if (urls.isEmpty()) {
        Q_EMIT errorOccurred(i18n("No valid tracks found in playlist: %1", path));
        return false;
    }

    addTracks(urls);
    return true;
}

void PlaylistModel::saveToDefault()
{
    if (m_tracks.isEmpty()) {
        return;
    }
    const QString path = defaultPlaylistPath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    writeM3U(path);
}

void PlaylistModel::loadFromDefault()
{
    const QString path = defaultPlaylistPath();
    if (!QFile::exists(path)) {
        return;
    }
    addTracks(parsePlaylist(path));
}

bool PlaylistModel::writeM3U(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Q_EMIT errorOccurred(i18n("Could not save playlist: %1", path));
        return false;
    }

    const bool utf8 = path.endsWith(QLatin1String(".m3u8"), Qt::CaseInsensitive);
    QTextStream out(&file);
    out.setEncoding(utf8 ? QStringConverter::Utf8 : QStringConverter::Latin1);

    out << QStringLiteral("#EXTM3U\n");

    const QDir baseDir = QFileInfo(path).dir();
    for (const Track &track : m_tracks) {
        const QString filePath = track.url.toLocalFile();
        const int seconds = static_cast<int>(track.duration / 1000);
        out << QStringLiteral("#EXTINF:%1,%2 - %3\n").arg(seconds).arg(track.artist, track.title);

        const QString relative = baseDir.relativeFilePath(filePath);
        const QString entry = relative.startsWith(QLatin1String("..")) ? filePath : relative;
        out << entry << '\n';
    }

    return true;
}

bool PlaylistModel::writePLS(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        Q_EMIT errorOccurred(i18n("Could not save playlist: %1", path));
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Latin1);

    out << QStringLiteral("[playlist]\n");

    const QDir baseDir = QFileInfo(path).dir();
    int number = 1;
    for (const Track &track : m_tracks) {
        const QString filePath = track.url.toLocalFile();
        const QString relative = baseDir.relativeFilePath(filePath);
        const QString entry = relative.startsWith(QLatin1String("..")) ? filePath : relative;

        out << QStringLiteral("File%1=%2\n").arg(number).arg(entry);
        out << QStringLiteral("Title%1=%2 - %3\n").arg(number).arg(track.artist, track.title);
        out << QStringLiteral("Length%1=%2\n").arg(number).arg(track.duration / 1000);
        ++number;
    }

    out << QStringLiteral("NumberOfEntries=%1\n").arg(m_tracks.size());
    out << QStringLiteral("Version=2\n");

    return true;
}

QList<QUrl> PlaylistModel::parsePlaylist(const QString &path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    const QString content = QString::fromUtf8(file.readAll());
    const QStringList lines = content.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    const QDir baseDir = QFileInfo(path).dir();
    const bool isPls = path.endsWith(QLatin1String(".pls"), Qt::CaseInsensitive);

    QList<QUrl> urls;
    for (const QString &rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (line.startsWith(QLatin1Char('#')) || line.startsWith(QLatin1Char('['))) {
            continue;
        }

        QString entry;
        if (isPls) {
            static const QRegularExpression fileEntry(QStringLiteral("^File\\d+=(.+)$"));
            const QRegularExpressionMatch match = fileEntry.match(line);
            if (!match.hasMatch()) {
                continue;
            }
            entry = match.captured(1).trimmed();
        } else {
            entry = line;
        }

        if (entry.isEmpty()) {
            continue;
        }

        QString filePath = entry;
        if (QDir::isRelativePath(filePath)) {
            filePath = baseDir.absoluteFilePath(filePath);
        }
        if (QFile::exists(filePath)) {
            urls.append(QUrl::fromLocalFile(QFileInfo(filePath).absoluteFilePath()));
        }
    }

    return urls;
}

Track PlaylistModel::extractTrackMetadata(const QUrl &url)
{
    Track track;
    track.url = url;

    QString path = url.toLocalFile();
    QFileInfo fileInfo(path);

    TagLib::FileRef file(path.toUtf8().constData());
    if (file.isNull()) {
        // Fallback to filename
        track.title = fileInfo.completeBaseName();
        track.artist = i18n("Unknown Artist");
        track.album = i18n("Unknown Album");
        return track;
    }

    TagLib::Tag *tag = file.tag();
    if (tag) {
        // Extract title
        if (tag->title().isEmpty()) {
            track.title = fileInfo.completeBaseName();
        } else {
            track.title = QString::fromStdString(tag->title().to8Bit(true));
        }

        // Extract artist
        if (tag->artist().isEmpty()) {
            track.artist = i18n("Unknown Artist");
        } else {
            track.artist = QString::fromStdString(tag->artist().to8Bit(true));
        }

        // Extract album
        if (tag->album().isEmpty()) {
            track.album = i18n("Unknown Album");
        } else {
            track.album = QString::fromStdString(tag->album().to8Bit(true));
        }
    } else {
        track.title = fileInfo.completeBaseName();
        track.artist = i18n("Unknown Artist");
        track.album = i18n("Unknown Album");
    }

    // Extract duration
    TagLib::AudioProperties *properties = file.audioProperties();
    if (properties) {
        track.duration = properties->lengthInMilliseconds();
    }

    // Extract album art
    TagLib::StringList complexKeys = file.complexPropertyKeys();
    if (complexKeys.contains("PICTURE")) {
        auto pictures = file.complexProperties("PICTURE");
        if (!pictures.isEmpty()) {
            const auto &picture = pictures[0];
            auto dataIt = picture.find("data");
            if (dataIt != picture.end()) {
                const TagLib::Variant &pictureData = dataIt->second;
                if (pictureData.type() == TagLib::Variant::ByteVector) {
                    TagLib::ByteVector data = pictureData.value<TagLib::ByteVector>();
                    if (!data.isEmpty()) {
                        // Determine file extension
                        QString extension = QStringLiteral(".jpg");
                        auto mimeIt = picture.find("mimeType");
                        if (mimeIt != picture.end()) {
                            QString mimeType = QString::fromStdString(mimeIt->second.value<TagLib::String>().to8Bit(true));
                            if (mimeType.contains(QStringLiteral("png"), Qt::CaseInsensitive)) {
                                extension = QStringLiteral(".png");
                            } else if (mimeType.contains(QStringLiteral("gif"), Qt::CaseInsensitive)) {
                                extension = QStringLiteral(".gif");
                            }
                        }

                        // Create temp file
                        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
                        QString tempTemplate = tempDir + QStringLiteral("/musik_playlist_art_XXXXXX") + extension;

                        QTemporaryFile tempFile(tempTemplate);
                        tempFile.setAutoRemove(false);

                        if (tempFile.open()) {
                            tempFile.write(data.data(), data.size());
                            QString tempPath = tempFile.fileName();
                            tempFile.close();

                            track.albumArtPath = QStringLiteral("file://") + tempPath;
                            m_tempArtPaths.append(tempPath);
                        }
                    }
                }
            }
        }
    }

    return track;
}

void PlaylistModel::cleanupTempFiles()
{
    for (const QString &path : m_tempArtPaths) {
        if (QFile::exists(path)) {
            QFile::remove(path);
        }
    }
    m_tempArtPaths.clear();
}

bool PlaylistModel::isAudioFile(const QString &path) const
{
    static const QStringList extensions = {
        QStringLiteral(".mp3"),
        QStringLiteral(".flac"),
        QStringLiteral(".ogg"),
        QStringLiteral(".wav"),
        QStringLiteral(".m4a"),
        QStringLiteral(".aac"),
        QStringLiteral(".wma"),
        QStringLiteral(".opus")
    };

    QString lowerPath = path.toLower();
    for (const QString &ext : extensions) {
        if (lowerPath.endsWith(ext)) {
            return true;
        }
    }
    return false;
}
