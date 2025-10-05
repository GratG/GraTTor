#include "client.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QtEndian>
#include <QTcpSocket>

const int BLOCK_SIZE = 16 * 1024; //16384

Client::Client(QObject* parent) : QObject(parent){
    qDebug() << "client created...";
    handshakeSent = false; handshakeRecieved = false;
    nextPacketLen = -1;
    tracker = new Tracker(this);
    socket = new QTcpSocket(this);
    //socket connections:
    connect(socket, &QTcpSocket::connected, this, &Client::tcpConnected);
    connect(socket, &QTcpSocket::disconnected, this, &Client::tcpDisconnected);
    connect(tracker, &Tracker::peersFound, this, &Client::connectPeers);
    //connect(tracker, &Tracker::replyFinished, this, &Client::handShake);



    connect(socket, &QTcpSocket::readyRead, this, &Client::readData);
}

bool Client::setTorrent(const QString &fileName, const QString &downloadPath)
{
    torrent = new Torrent(fileName);
    torrent->extractBEncode();
    downloadDest = downloadPath;

    fileManager = new FileManager();
    fileManager->setPath(downloadPath);
    fileManager->setTorrent(torrent);
    fileManager->start();
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

void Client::tcpDisconnected()
{
    qDebug() << "Tcp socket is disconnected";
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
    //QByteArray response = socket->readAll();
    qDebug() << "message recieved";
    if(!handshakeRecieved){
        if(socket->bytesAvailable() < 68){
            return;
        }
        qDebug() << "handshake recieved...";
        qDebug() << "validating handshake...";

        QByteArray protoId = socket->read(1);
        if(protoId != 19)
            return;

        QByteArray protoStr = socket->read(19);
        if(protoStr != "BitTorrent protocol")
            return;
        QByteArray reserved = socket->read(8);

        QByteArray info = socket->read(20);
        if(info != torrent->getSHA1())
            return;

        QByteArray peerID = socket->read(20);

        handshakeRecieved = true;
        qDebug() << info;
    } else{
        //TODO ensure packet length is right
        if(nextPacketLen == -1){
            if(socket->bytesAvailable() < 4)
                return;

            nextPacketLen = qFromBigEndian<qint32>(socket->read(4));
            qDebug() << "Packet Length: " << nextPacketLen;
        }

        if(nextPacketLen == 0){
            nextPacketLen = -1;

        }

        if(socket->bytesAvailable() < nextPacketLen){
            return;
        }

        QByteArray packet = socket->read(nextPacketLen);
        if(packet.size() != nextPacketLen){
            qDebug() << "abort";
            socket->abort();
            return;
        }

        switch(packet[0]){
            case chokePacket:
                qDebug() << "choke packet";
                break;
            case unchokePacket:
                qDebug() << "unchoke packet";
                testRequest();
                break;
            case interestedPacket:{
                qDebug() << "interested packet";
                break;
            }
            case notInterestedPacket:
                qDebug() << "not interested packet";
                break;
            case havePacket:
                qDebug() << "have packet";
                break;
            case bitfieldPacket:{
                qDebug() << "bitfield packet";
                //send interested message
                QByteArray packet;
                QDataStream out(&packet, QIODevice::WriteOnly);
                out << (qint32)1;
                packet.append(char(2));
                socket->write(packet);
                socket->flush();

                qDebug() << "Sent INTERESTED message:" << packet.toHex();

                break;
            }
            case requestPacket:
                qDebug() << "request packet";
                break;
            case piecePacket:
                qDebug() << "piece packet";
                testReceive(packet);
                break;
            case cancelPacket:
                qDebug() << "cancel packet";
                break;
            default:
                qDebug() << "unsupported packet";
                break;
        }
        nextPacketLen = -1;
    }}




void Client::testRequest()
{
    QByteArray request;
    QDataStream out(&request, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);

    // length prefix (13)
    out << (quint32)13;

    // message ID (6)
    out << char(6);

    //out.setByteOrder(QDataStream::BigEndian);
    // piece index (0), begin (0), length (16384)
    out << (quint32)0;
    out << (quint32)0;
    out << (quint32)16384;
    socket->write(request);
    socket->flush();

    qDebug() << "Sent request:" << request.toHex();
}

void Client::testReceive(QByteArray &packet)
{
    //TODO abort if invalid size
    qDebug() << packet.size();
    packet.remove(0, 1);
    int index = packet.left(4).toInt();
    packet.remove(0,4);
    int begin = packet.left(4).toInt();
    packet.remove(0,4);
    qDebug() <<"packet contents: " <<  packet;
    qDebug() << "Packet length: " << packet.size();

    fileManager->writeBlock(index, begin, packet);
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

