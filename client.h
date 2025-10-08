#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QBitArray>
#include <QUrl>
#include <QTcpSocket>

#include "torrent.h"
#include "tracker.h"
#include "filemanager.h"

class Torrent;
class Tracker;
class FileManager;
class Client : public QObject {
    Q_OBJECT
    public:
        Client(QObject* parent = nullptr);

        bool setTorrent(const QString &fileName, const QString &downloadPath = "");
        bool sendRequest(int pieceIndex, int offset, int length);
    private:
        Torrent *torrent;
        FileManager *fileManager;
        Tracker *tracker;
        QTcpSocket *socket;
        QList<QVariant> availablePeers;

        QString downloadDest;
        int nextPacketLen;

        void tcpConnected();
        void tcpDisconnected();
        void sendHandshake();
        void readData();

        //packet related functions
        void choked();
        void unchoked();

        void bitfieldReceived(QByteArray &packet);
        void packetReceived(QByteArray &packet);

        //
        void initDownload();

        //TODO refactor this:
        void testRequest();
        void testReceive(QByteArray &packet);

        bool handshakeSent;
        bool handshakeRecieved;

        QBitArray availablePieces;
        enum patcketType{
            chokePacket = 0,
            unchokePacket = 1,
            interestedPacket = 2,
            notInterestedPacket = 3,
            havePacket = 4,
            bitfieldPacket = 5,
            requestPacket = 6,
            piecePacket = 7,
            cancelPacket = 8
        };

    public slots:
        void connectPeers(QList<QPair<QString, quint16>> peerList);

    signals:
        void beginRequest();

};

#endif // CLIENT_H
