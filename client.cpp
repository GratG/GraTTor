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
    clientInterested = false;
    nextPacketLen = -1;
    tracker = new Tracker(this);
    createSocket();
    //tracker connections
    connect(tracker, &Tracker::peersFound, this, &Client::connectPeers);

}

QString Client::getTorrName()
{
    return torrent->getFileName();
}

void Client::createSocket()
{
    socket = new QTcpSocket(this);
    timeoutTimer = new QTimer();
    timeoutTimer->setSingleShot(true);
    //socket connections:
    connect(socket, &QTcpSocket::connected, this, &Client::tcpConnected);
    connect(socket, &QTcpSocket::disconnected, this, &Client::tcpDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &Client::readData);
    connect(timeoutTimer, &QTimer::timeout, this, &Client::timeoutSocket);
}


bool Client::setTorrent(const QString &fileName, const QString &downloadPath)
{
    torrent = new Torrent(fileName);
    torrent->extractBEncode();
    downloadDest = downloadPath;

    myPieces.resize(torrent->getPieceCount());
    myPieces.fill(false);
    peerPieces.resize(torrent->getPieceCount());
    peerPieces.fill(false);

    fileManager = new FileManager(this);
    fileManager->setPath(downloadPath);
    fileManager->setTorrent(torrent);


    tracker->addTorrent(torrent);
    tracker->start();

    //filemanager connections
    connect(this, &Client::beginRequest, fileManager, &FileManager::peerUnchoked);
    connect(fileManager, &FileManager::validPiece ,this, &Client::pieceUpdate);

    fileManager->start(QThread::LowestPriority);
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



    return true;
}



void Client::tcpConnected()
{
    //qDebug() << "Tcp socket is connected";


    if(!handshakeSent){
        //qDebug() << "sending handshake";
        sendHandshake();
    }
}

void Client::tcpDisconnected()
{
    qDebug() << "Tcp socket is disconnected, attempting reconnect";
    handshakeSent = false;
    handshakeRecieved = false;
    clientInterested = false;
    nextPacketLen = -1;
    createSocket();
    reconnectPeer();
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

    timeoutTimer->stop();
    //qDebug() << "message recieved";

    if(!handshakeRecieved){
        if(socket->bytesAvailable() < 68){
            return;
        }
        qDebug() << "handshake recieved...";

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

    } else{
        //TODO ensure packet length is right
        if(nextPacketLen == -1){
            if(socket->bytesAvailable() < 4)
                return;

            nextPacketLen = qFromBigEndian<qint32>(socket->read(4));

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
                //qDebug() << "choke packet";
                choked();
                break;
            case unchokePacket:
                //qDebug() << "unchoke packet: " << packet.toHex();

                unchoked();
                break;
            case interestedPacket:{
                //qDebug() << "interested packet";
                break;
            }
            case notInterestedPacket:
                //qDebug() << "not interested packet";
                break;
            case havePacket:{
                //qDebug() << "have packet" << packet.toHex();

                quint32 index = qFromBigEndian<quint32>(&packet.data()[0]);
                qDebug() << "piece index: " << index;
                if (index < quint32(peerPieces.size())) {
                    peerPieces.setBit(int(index));
                }
                peerPieceUpdate();
                break;
            }
            case bitfieldPacket:{
                //qDebug() << "bitfield packet";
                //send interested message
                bitfieldReceived(packet);
                sendInterested();
                break;
            }
            case requestPacket:
                //qDebug() << "request packet";
                break;
            case piecePacket:
                //qDebug() << "piece packet";
                packetReceived(packet);
                break;
            case cancelPacket:
                //qDebug() << "cancel packet";
                break;
            default:
                //qDebug() << "unsupported packet";
                break;
        }
        nextPacketLen = -1;
        timeoutTimer->start(50000);
    }}


void Client::packetReceived(QByteArray &packet)
{
    //TODO abort if invalid size
    //qDebug() << packet.size();
    QDataStream stream(&packet, QIODevice::ReadOnly);
    stream.setByteOrder(QDataStream::BigEndian);
    quint32 index, begin;

    stream >> index;
    stream >> begin;
    //qDebug() << "packet received info: index: " << index << " begin: " << begin;
    //qDebug() <<"packet contents: " <<  packet;
    fileManager->blockRecieved(index, begin/BLOCK_SIZE);
    qDebug() << "Packet length: " << packet.size();
    packet.remove(0,8);
    fileManager->writeRequest(index, begin, packet);
    if(fileManager->remainingBlocks(index) == 0){
        //validate piece if it is finished downloading
        fileManager->addVerifyPiece(index);
    }

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
    if(clientInterested){
        testRequest();
        emit beginRequest();
    } else{
        sendInterested();
    }


}

void Client::sendHave(quint32 &index)
{
    //TODO
}


void Client::peerPieceUpdate()
{
    //find a piece in peerPieces, then send request
    bool interested;
    for(int i = 0; i < peerPieces.size(); i++){
        if(peerPieces.testBit(i)){
            if(!myPieces.testBit(i)){
                interested = true;
                testRequest();
            }
        }
    }

}

void Client::bitfieldReceived(QByteArray &packet)
{
    //figure out which pieces are available from the peer
    peerPieces.clear();
    for(int i = 0; i < packet.size(); i++){
        quint8 byte = static_cast<quint8>(packet[i]);
        for(int bit = 0; bit < 8; bit++){
            int pieceIndex = i * 8 + (7-bit);
            if(peerPieces.size() <= pieceIndex)
                peerPieces.resize(pieceIndex + 1);
            if(byte & (1<<bit))
                peerPieces.setBit(pieceIndex, true);
        }
    }

    //qDebug() << "peer pieces: " << peerPieces;
}

void Client::sendInterested()
{

    QByteArray outPacket;
    QDataStream out(&outPacket, QIODevice::WriteOnly);
    out << (qint32)1;
    outPacket.append(char(2));
    //qDebug() << "Sent INTERESTED message:" << outPacket.toHex();

    socket->write(outPacket);

    socket->flush();
    clientInterested = true;
    timeoutTimer->start(500000);

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

    int nextPiece = fileManager->selectNextPiece(peerPieces);

    //TODO if piece completed (verified) update myPieces
    if(nextPiece != -1){
        int nextBlock = fileManager->selectBlock(nextPiece);
        int blockLength = fileManager->calcBlockLength(nextPiece, nextBlock);
        //qDebug() << "Piece index: " << nextPiece
        //         << " block index: " << nextBlock
        //         << " block Length: " << blockLength;
        fileManager->blockRequested(nextPiece, nextBlock);
        sendRequest(nextPiece, nextBlock * BLOCK_SIZE, blockLength);

    } else{
        //sendInterested();
        if(myPieces.count(true) == torrent->getPieceHashes().size()){
            qDebug() << "TORRENT COMPLETED";
        }
    }


}

void Client::testReceive(QByteArray &packet)
{

}

void Client::connectPeers(QList<QPair<QString, quint16>> peerList)
{
    //TODO peerList queue?
    int random = QRandomGenerator::global()->bounded(peerList.size());

    for (auto &p : peerList) {
        //qDebug() << "Peer:" << p.first << ":" << p.second;
    }

    socket->connectToHost(peerList[random].first, peerList[random].second);

    if(!socket->waitForConnected(5000)){
        qWarning() << "Could not connect: " << socket->errorString();
    }

}

void Client::pieceUpdate(quint32 index)
{
    myPieces.setBit(index, true);
    qDebug() << myPieces.count(true) << "/" << torrent->getPieceHashes().size();
    sendHave(index);

    double percent = ((myPieces.count(true) * 100) / myPieces.size());
    emit updateProg(percent);

}

void Client::timeoutSocket()
{
    qDebug() << "TIMEOUT - SOCKET ABORTED";
    socket->abort();

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
    timeoutTimer->start(500000);
}


