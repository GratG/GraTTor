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
#include <QRandomGenerator>

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
    connect(socket, &QTcpSocket::readyRead, this, &Client::readData);

    //tracker connections
    connect(tracker, &Tracker::peersFound, this, &Client::connectPeers);


}

bool Client::setTorrent(const QString &fileName, const QString &downloadPath)
{
    torrent = new Torrent(fileName);
    torrent->extractBEncode();
    downloadDest = downloadPath;

    fileManager = new FileManager();
    fileManager->setPath(downloadPath);
    fileManager->setTorrent(torrent);

    fileManager->start(QThread::LowestPriority);
    tracker->addTorrent(torrent);
    tracker->start();

    //filemanager connections
    connect(this, &Client::beginRequest, fileManager, &FileManager::peerUnchoked);

    return true;
}

bool Client::sendRequest(int pieceIndex, int offset, int length)
{

    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);

    quint32 msgLength = 13;
    quint8 msgId = 6;

    out << msgLength;
    out << msgId;
    out << (quint32)pieceIndex;
    out << (quint32)offset;
    out << (quint32)length;

    socket->write(packet);
    socket->flush();

    qDebug() << "Sent request:" << packet.toHex();

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
    qDebug() << "Tcp socket is disconnected, attempting reconnect";
    handshakeSent, handshakeSent = false;
    //reconnectPeer();
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

        int packetId = packet[0];
        packet.remove(0,1);
        switch(packetId){
            case chokePacket:
                qDebug() << "choke packet";
                choked();
                break;
            case unchokePacket:
                qDebug() << "unchoke packet";
                unchoked();
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
                bitfieldReceived(packet);
                QByteArray outPacket;
                QDataStream out(&outPacket, QIODevice::WriteOnly);
                out << (qint32)1;
                outPacket.append(char(2));
                socket->write(outPacket);
                socket->flush();

                qDebug() << "Sent INTERESTED message:" << outPacket.toHex();

                break;
            }
            case requestPacket:
                qDebug() << "request packet";
                break;
            case piecePacket:
                qDebug() << "piece packet";
                packetReceived(packet);
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


void Client::packetReceived(QByteArray &packet)
{
    //TODO abort if invalid size
    //qDebug() << packet.size();
    QDataStream stream(&packet, QIODevice::ReadOnly);
    quint32 index, begin;
    stream >> index;
    stream >> begin;
    qDebug() << "packet received info: index: " << index << " begin: " << begin;
    fileManager->blockRecieved(index, begin/BLOCK_SIZE);
    //qDebug() <<"packet contents: " <<  packet;
    qDebug() << "Packet length: " << packet.size();

    //qDebug() << "packet index: " << index <<" packet offset: " << begin;
    fileManager->writeRequest(index, begin, packet);
    testRequest();
}

void Client::reqNextPiece()
{




}

void Client::choked()
{
    //TODO choke and stop current block/piece writing
}

void Client::unchoked()
{
    //we are free to request blocks
    //TODO start requesting blocks/pieces through filemanager
    testRequest();
    emit beginRequest();

}

void Client::bitfieldReceived(QByteArray &packet)
{
    //figure out which pieces are available
    availablePieces.clear();
    for(int i = 0; i < packet.size(); i++){
        quint8 byte = static_cast<quint8>(packet[i]);
        for(int bit = 0; bit < 8; bit++){
            int pieceIndex = i * 8 + (7-bit);
            if(availablePieces.size() <= pieceIndex)
                availablePieces.resize(pieceIndex + 1);
            if(byte & (1<<bit))
                availablePieces.setBit(pieceIndex, true);
        }
    }

    qDebug() << "available pieces: " << availablePieces;
}

void Client::initDownload()
{
    //emit peerUnchoke;
}




void Client::testRequest()
{
    QByteArray request;
    QDataStream out(&request, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);

    int nextPiece = fileManager->selectNextPiece(availablePieces);


    if(nextPiece != -1){
        int nextBlock = fileManager->selectBlock(nextPiece);
        int blockLength = fileManager->calcBlockLength(nextPiece, nextBlock);
        qDebug() << "Piece index: " << nextPiece
                 << " block index: " << nextBlock
                 << " block Length: " << blockLength;
        fileManager->blockRequested(nextPiece, nextBlock);
        sendRequest(nextPiece, nextBlock * BLOCK_SIZE, blockLength);

    }
    //sendRequest(nextPiece, 1 * BLOCK_SIZE, BLOCK_SIZE);

    //sendRequest(2, 0 * BLOCK_SIZE, BLOCK_SIZE);


}

void Client::testReceive(QByteArray &packet)
{

}

void Client::connectPeers(QList<QPair<QString, quint16>> peerList)
{
    //TODO peerList queue?
    int random = QRandomGenerator::global()->bounded(peerList.size());
    for (auto &p : peerList) {
        qDebug() << "Peer:" << p.first << ":" << p.second;
    }

    socket->connectToHost(peerList[random].first, peerList[random].second);

    if(!socket->waitForConnected(5000)){
        qWarning() << "Could not connect: " << socket->errorString();
    }

}

void Client::reconnectPeer()
{
    //todo ensure peerlist is populated
    QList<QPair<QString, quint16>> peerList = tracker->getPeerList();
    int random = QRandomGenerator::global()->bounded(peerList.size());

    socket->connectToHost(peerList[random].first, peerList[random].second);

    if(!socket->waitForConnected(5000)){
        qWarning() << "Could not connect: " << socket->errorString();
    }
}


