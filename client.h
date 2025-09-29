#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QUrl>
#include <QTcpSocket>

#include "torrent.h"
#include "tracker.h"

class Torrent;
class Tracker;
class Client : public QObject {
    Q_OBJECT
    public:
        Client(QObject* parent = nullptr);

        bool setTorrent(const QString &fileName);
    private:
        Torrent *torrent;
        Tracker *tracker;
        QTcpSocket *socket;
        QList<QVariant> availablePeers;

        void tcpConnected();
        void sendHandshake();
        void readData();

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
