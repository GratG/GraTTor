#ifndef TORRENT_H
#define TORRENT_H

#include "bencoding.h"
#include "tracker.h"
#include <QObject>
#include <QString>
#include <QVariant>

struct TorrentInfo {
    quint64 length;
    QString name;
    quint64 pieceLength;
    QByteArray pieces;
};

struct MetaInfo {
    QString announce;
    TorrentInfo info;
    QByteArray sha1;
    QList<QByteArray> pieceHashes;
};

class FileItem {
    public:
        QString path;
        uint64_t size;
        uint64_t offset;
};

class Torrent {
    public:
        Torrent(QString fileDir);
        bool extractBEncode();

        QString getAnnounce(int index) const;
        QByteArray getPieces() const;
        QList<QByteArray> getPieceHashes() const;
        QString getAnnounce() const;
        QByteArray getSHA1() const;
        uint64_t getLength() const;
        QString getFileName();
        qint64 getPieceLength() const;
    private:

        QString fileName;
        QString fileDir;
        QString downloadDir;
        //meta data from .torrent file
        MetaInfo metaInfo;

        QList<QString> announceList;

        QString comment;
        QString createdBy;
        //file info
        QList<FileItem> files;
        //tracker info
        //QList<Tracker> trackers;

};

#endif // TORRENT_H
