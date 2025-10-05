#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
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

        //TODO refactor this:
        void testRequest();
        void testReceive(QByteArray &packet);

        bool handshakeSent;
        bool handshakeRecieved;

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

};

#endif // CLIENT_H
