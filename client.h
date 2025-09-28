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


    public slots:
        void handShake();
        void connectPeers(QList<QPair<QString, quint16>> peerList);

};

#endif // CLIENT_H
