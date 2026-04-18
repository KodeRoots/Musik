// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2026 Musik Authors

#include "recentfilesmodel.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QFile>
#include <QFileInfo>
#include <QDebug>

RecentFilesModel::RecentFilesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    loadFromConfig();
}

RecentFilesModel::~RecentFilesModel()
{
    saveToConfig();
}

int RecentFilesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_files.size();
}

QVariant RecentFilesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_files.size()) {
        return QVariant();
    }

    const RecentFile &file = m_files.at(index.row());

    switch (role) {
    case UrlRole:
        return file.url;
    case DisplayNameRole:
        return file.displayName;
    case AlbumArtRole:
        return file.albumArtPath;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> RecentFilesModel::roleNames() const
{
    return {
        {UrlRole, "url"},
        {DisplayNameRole, "displayName"},
        {AlbumArtRole, "albumArt"}
    };
}

int RecentFilesModel::count() const
{
    return m_files.size();
}

void RecentFilesModel::addFile(const QUrl &url)
{
    qDebug() << "RecentFilesModel::addFile - url:" << url.toString();
    if (!url.isLocalFile()) {
        qDebug() << "  -> rejected: not local file";
        return;
    }

    QString path = url.toLocalFile();
    bool exists = QFile::exists(path);
    qDebug() << "  -> path:" << path << "exists:" << exists;
    if (!exists) {
        qDebug() << "  -> rejected: file does not exist";
        return;
    }

    // Remove duplicate if already in list
    for (int i = 0; i < m_files.size(); ++i) {
        if (m_files.at(i).url == url) {
            beginRemoveRows(QModelIndex(), i, i);
            m_files.removeAt(i);
            endRemoveRows();
            break;
        }
    }

    // Insert at front
    RecentFile entry;
    entry.url = url;
    entry.displayName = extractDisplayName(url);
    entry.albumArtPath = QString();

    beginInsertRows(QModelIndex(), 0, 0);
    m_files.prepend(entry);
    endInsertRows();

    // Cap at max
    while (m_files.size() > MaxRecentFiles) {
        int last = m_files.size() - 1;
        beginRemoveRows(QModelIndex(), last, last);
        m_files.removeAt(last);
        endRemoveRows();
    }

    Q_EMIT countChanged();
    saveToConfig();
    qDebug() << "RecentFilesModel::addFile - done, total files:" << m_files.size();
}

void RecentFilesModel::removeFile(int index)
{
    if (index < 0 || index >= m_files.size()) {
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_files.removeAt(index);
    endRemoveRows();

    Q_EMIT countChanged();
    saveToConfig();
}

void RecentFilesModel::clear()
{
    if (m_files.isEmpty()) {
        return;
    }

    beginResetModel();
    m_files.clear();
    endResetModel();

    Q_EMIT countChanged();
    saveToConfig();
}

void RecentFilesModel::loadFromConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(QStringLiteral("RecentFiles"));

    int count = group.readEntry(QStringLiteral("Count"), 0);
    qDebug() << "RecentFilesModel::loadFromConfig - Count from config:" << count;

    for (int i = 0; i < count; ++i) {
        QString urlStr = group.readEntry(QStringLiteral("Url%1").arg(i), QString());
        qDebug() << "  Url" << i << ":" << urlStr;

        if (urlStr.isEmpty()) {
            qDebug() << "    -> skipping: empty";
            continue;
        }

        QUrl url(urlStr);
        if (!url.isLocalFile()) {
            qDebug() << "    -> skipping: not local file";
            continue;
        }

        QString path = url.toLocalFile();
        bool exists = QFile::exists(path);
        qDebug() << "    -> path:" << path << "exists:" << exists;
        if (!exists) {
            qDebug() << "    -> skipping: file does not exist";
            continue;
        }

        RecentFile entry;
        entry.url = url;
        entry.displayName = extractDisplayName(url);
        entry.albumArtPath = QString();

        m_files.append(entry);
        qDebug() << "    -> added:" << entry.displayName;
    }

    qDebug() << "RecentFilesModel::loadFromConfig - total loaded:" << m_files.size();
    Q_EMIT countChanged();
}

void RecentFilesModel::saveToConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(QStringLiteral("RecentFiles"));

    // Clear existing entries
    int oldCount = group.readEntry(QStringLiteral("Count"), 0);
    for (int i = 0; i < oldCount; ++i) {
        group.deleteEntry(QStringLiteral("Url%1").arg(i));
    }

    // Write new entries
    group.writeEntry(QStringLiteral("Count"), m_files.size());
    for (int i = 0; i < m_files.size(); ++i) {
        group.writeEntry(QStringLiteral("Url%1").arg(i), m_files.at(i).url.toString());
    }

    group.sync();
}

QString RecentFilesModel::extractDisplayName(const QUrl &url) const
{
    if (!url.isLocalFile()) {
        return url.fileName();
    }

    QFileInfo info(url.toLocalFile());
    return info.completeBaseName();
}
