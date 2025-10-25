#ifndef TRACKER_H
#define TRACKER_H

#include <QObject>
#include <QNetworkAccessManager>

#include "torrent.h"
#include "client.h"

class Torrent;

class Tracker : public QObject {
    Q_OBJECT
    public:
        Tracker(QObject *parent = nullptr);
        void addTorrent(Torrent *t);
        void start();
        QList<QPair<QString, quint16>> getPeerList();

    private:
        QString addr;
        Torrent *torrent;
        QNetworkAccessManager httpManager;
        QList<QPair<QString, quint16>> peerList;

    private slots:
        void onReplyFinished(QNetworkReply *reply);
        void fetchPeerList();

    signals:
        void replyFinished();
        void peersFound(QList<QPair<QString, quint16>> peerList);






};

#endif // TRACKER_H
