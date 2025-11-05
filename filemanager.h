#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QFile>
#include <QList>
#include <QThread>
#include <QMutex>
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
class FileManager: public QThread {
    Q_OBJECT
    public:
        FileManager(QObject *parent = nullptr);
        ~FileManager();
        void setTorrent(Torrent *t);
        void setPath(const QString &p);

        int selectNextPiece(QBitArray &availablePieces);
        int selectBlock(int i);
        int calcBlockLength(int p, int b);
        int remainingBlocks(const int index) const;
        void requestNextBlock(Client *c, int pieceIndex);
        void blockRequested(int p, int b);
        bool blockRecieved(int p, int b);
        void writeRequest(quint32 &index, quint32 &offset, QByteArray &data);

        void addVerifyPiece(quint32 index);
        void resetInvalidPiece(quint32 index);

    private:
        bool quit;
        Torrent *torrent;
        QString downloadDest;
        QList<QFile*> fileList;
        bool createFiles();

        QList<Piece> pieces;
        quint64 pieceLength;
        quint64 totalSize;

        QMutex mutex;

        struct WriteRequest {
            quint32 pieceIndex;
            quint32 offset;
            QByteArray data;
        };
        //struct ReadRequest {
        //    qint32 pieceIndex;
        //    qint32 offset;
        //    qint32 length;
        //    qint32 id;
        //};
        QList<WriteRequest> writeRequests;
        QList<quint32> verifyRequests;
        bool writeBlock(quint32 &index, quint32 &offset, QByteArray &data);
        QByteArray readBlock(quint32 index, quint32 offset, quint32 length);
        bool verifyPiece(quint32 index);

        void verifyList();

    public slots:
        void peerUnchoked();

    signals:
        void validPiece(quint32 index);
    protected:
        void run() override;


};

#endif // FILEMANAGER_H
