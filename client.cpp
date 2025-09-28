#include "client.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>

#include <QTcpSocket>

Client::Client(QObject* parent) : QObject(parent){
    qDebug() << "client created...";
    tracker = new Tracker(this);
    socket = new QTcpSocket(this);
    //socket connections:
    connect(socket, &QTcpSocket::connected, this, &Client::tcpConnected);

    connect(tracker, &Tracker::peersFound, this, &Client::connectPeers);
    //connect(tracker, &Tracker::replyFinished, this, &Client::handShake);
}

bool Client::setTorrent(const QString &fileName)
{
    torrent = new Torrent(fileName);
    torrent->extractBEncode();

    tracker->addTorrent(torrent);
    tracker->start();
    return true;
}

void Client::tcpConnected()
{
    qDebug() << "Tcp socket is connected";
    //QByteArray handshakeStr = buildHandshake();
}

void Client::handShake()
{
    QByteArray handshakeHash = torrent->getSHA1();
    //TODO generate peerID
    QString peerID = "12345678901234567890";

}

void Client::connectPeers(QList<QPair<QString, quint16>> peerList)
{

    for (auto &p : peerList) {
        qDebug() << "Peer:" << p.first << ":" << p.second;
    }

    socket->connectToHost(peerList[1].first, peerList[1].second);

    if(!socket->waitForConnected(10000)){
        qWarning() << "Could not connect: " << socket->errorString();
    }

}

