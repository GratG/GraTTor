#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QBitArray>
#include <QUrl>
#include <QTcpSocket>
#include <QTimer>
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

        QString getTorrName();

        bool setTorrent(const QString &fileName, const QString &downloadPath = "");
        bool sendRequest(int pieceIndex, int offset, int length);
    private:
        Torrent *torrent;
        FileManager *fileManager;
        Tracker *tracker;
        QTcpSocket *socket;
        QList<QVariant> availablePeers;
        QTimer *timeoutTimer;

        QString downloadDest;
        quint32 nextPacketLen;

        void createSocket();
        void tcpConnected();
        void tcpDisconnected();
        void sendHandshake();
        void readData();
        void reconnectPeer();
        //packet related functions
        void choked();
        void unchoked();

        void sendHave(quint32 &index);
        void bitfieldReceived(QByteArray &packet);
        void sendInterested();
        bool clientInterested;
        void packetReceived(QByteArray &packet);
        void reqNextPiece();

        void peerPieceUpdate();
        //
        void initDownload();

        //TODO refactor this:
        void testRequest();
        void testReceive(QByteArray &packet);

        bool handshakeSent;
        bool handshakeRecieved;

        QBitArray myPieces;
        QBitArray peerPieces;
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
        void pieceUpdate(quint32 index);
        void timeoutSocket();

    signals:
        void beginRequest();
        void updateProg(double percent);

};

#endif // CLIENT_H
