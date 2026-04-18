// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2026 Musik Authors

#ifndef RECENTFILESMODEL_H
#define RECENTFILESMODEL_H

#include <QAbstractListModel>
#include <QFileInfo>
#include <QQmlEngine>
#include <QUrl>

struct RecentFile {
    QUrl url;
    QString displayName;
    QString albumArtPath;
};

class RecentFilesModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        UrlRole = Qt::UserRole + 1,
        DisplayNameRole,
        AlbumArtRole
    };

    explicit RecentFilesModel(QObject *parent = nullptr);
    ~RecentFilesModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const;

    Q_INVOKABLE void addFile(const QUrl &url);
    Q_INVOKABLE void removeFile(int index);
    Q_INVOKABLE void clear();

    static RecentFilesModel *create(QQmlEngine *, QJSEngine *)
    {
        return new RecentFilesModel;
    }

Q_SIGNALS:
    void countChanged();

private:
    void loadFromConfig();
    void saveToConfig();
    QString extractDisplayName(const QUrl &url) const;
    QString extractAlbumArt(const QString &path);
    void cleanupTempFiles();

    QList<RecentFile> m_files;
    QStringList m_tempArtPaths;
    static constexpr int MaxRecentFiles = 10;
};

#endif // RECENTFILESMODEL_H
