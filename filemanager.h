#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QFile>
#include <QList>

#include "torrent.h"
#include "client.h"

//const int BLOCK_SIZE = 16 * 1024; //16384

struct Block{
    bool requested = false;
    bool received = false;
};

struct Piece {
    QByteArray hash;
    QList<Block> blocks;
    bool completed = false;
};


class Torrent;
class Client;
class FileManager: public QObject {
    Q_OBJECT
    public:
        FileManager();
        void start();
        void setTorrent(Torrent *t);
        void setPath(const QString &p);

        int selectNextPiece(QBitArray &availablePieces);
        int selectBlock(int i);
        void requestNextBlock(Client *c, int pieceIndex);
        bool writeBlock(quint32 &index, quint32 &offset, QByteArray &data);


    private:
        Torrent *torrent;
        QString downloadDest;
        QList<QFile*> fileList;
        bool createFiles();

        QList<Piece> pieces;
        quint64 pieceLength;
        quint64 totalSize;

    public slots:
        void peerUnchoked();



};

#endif // FILEMANAGER_H
