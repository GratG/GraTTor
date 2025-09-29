#include "client.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>

#include <QTcpSocket>

Client::Client(QObject* parent) : QObject(parent){
    qDebug() << "client created...";
    handshakeSent = false; handshakeRecieved = false;
    tracker = new Tracker(this);
    socket = new QTcpSocket(this);
    //socket connections:
    connect(socket, &QTcpSocket::connected, this, &Client::tcpConnected);

    connect(tracker, &Tracker::peersFound, this, &Client::connectPeers);
    //connect(tracker, &Tracker::replyFinished, this, &Client::handShake);



    connect(socket, &QTcpSocket::readyRead, this, &Client::readData);
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
    if(!handshakeSent){
        sendHandshake();
    }
}

void Client::sendHandshake()
{
    handshakeSent = true;
    QByteArray infoHash = torrent->getSHA1();
    //TODO generate peerID
    QString peerID = "12345678901234567890";

    QByteArray handshake;
    handshake.append(char(19));
    handshake.append("BitTorrent protocol");
    handshake.append(QByteArray(8,0));
    handshake.append(infoHash);
    handshake.append(peerID.toUtf8());

    socket->write(handshake);
    socket->flush();


}

void Client::readData(){
    QByteArray response = socket->readAll();
    qDebug() << "message recieved";
    if(!handshakeRecieved){
        if(response.size() < 68){
            return;
        }
        qDebug() << "handshake recieved...";
        qDebug() << "validating handshake...";

        QByteArray protoId = response.mid(0,1);
        if(protoId != 19)
            return;
        QByteArray protoStr = response.mid(1, 19);
        if(protoStr != "BitTorrent protocol")
            return;
        QByteArray reserved = response.mid(20, 8);
        QByteArray info = response.mid(28, 20);
        if(info != torrent->getSHA1())
            return;
        QByteArray peerID = response.mid(48, 20);

        handshakeRecieved = true;
        qDebug() << info;
    } else{
        //TODO ensure packet length is right
        qDebug() << response;
        switch(response[4]){
            case chokePacket:
                qDebug() << "choke packet";
                break;
            case unchokePacket:
                qDebug() << "unchoke packet";
                break;
            case interestedPacket:
                qDebug() << "interested packet";
                break;
            case notInterestedPacket:
                qDebug() << "not interested packet";
                break;
            case havePacket:
                qDebug() << "have packet";
                break;
            case bitfieldPacket:
                qDebug() << "bitfield packet";
                break;
            case requestPacket:
                qDebug() << "request packet";
                break;
            case piecePacket:
                qDebug() << "piece packet";
                break;
            case cancelPacket:
                qDebug() << "cancel packet";
                break;
            default:
                qDebug() << "unsupported packet";
                break;
        }
    }


}

void Client::connectPeers(QList<QPair<QString, quint16>> peerList)
{
    //TODO peerList queue?
    for (auto &p : peerList) {
        qDebug() << "Peer:" << p.first << ":" << p.second;
    }

    qDebug() << peerList[2].second;
    socket->connectToHost(peerList[2].first, peerList[2].second);

    if(!socket->waitForConnected(5000)){
        qWarning() << "Could not connect: " << socket->errorString();
    }

}

