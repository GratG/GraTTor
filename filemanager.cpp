#include "filemanager.h"

#include <QDebug>


const int BLOCK_SIZE = 16 * 1024; //16384

FileManager::FileManager(QObject *parent): QThread(parent){
    quit = false;
}



FileManager::~FileManager()
{
    quit = true;
}

void FileManager::run()
{
    if(!createFiles()){
        return;
    }

    do{

        mutex.lock();
        QList<WriteRequest> newWriteRequests = writeRequests;
        writeRequests.clear();
        while(!quit && !newWriteRequests.isEmpty()){
            qDebug() << "writing block...";
            WriteRequest request = newWriteRequests.takeFirst();
            writeBlock(request.pieceIndex, request.offset, request.data);
        }
        mutex.unlock();
        //manage verification requests
        if(!verifyRequests.isEmpty()){
            for(int i = 0; i < verifyRequests.size(); i ++){
                verifyPiece(verifyRequests[i]);
            }
            verifyRequests.clear();
        }


    } while(!quit);

}

void FileManager::setTorrent(Torrent *t)
{
    torrent = t;
    pieceLength = t->getPieceLength();
    totalSize = t->getLength();

    QList<QByteArray> pieceHashes = t->getPieceHashes();
    pieces.resize(pieceHashes.size());
    for(int i = 0; i < pieceHashes.size(); i++){
        pieces[i].hash = pieceHashes[i];
        int numBlocks = (pieceLength + BLOCK_SIZE - 1) / BLOCK_SIZE;

        pieces[i].blocks.resize(numBlocks);
    }

}

void FileManager::setPath(const QString &p)
{
    downloadDest = p;
}

int FileManager::selectNextPiece(QBitArray &availablePieces)
{

    for (int i = 0; i < pieces.size(); i++){
        if(pieces[i].completed){
            continue;
        }
        if(availablePieces.testBit(i)){
            qDebug() << "available piece found at " << i;
            return i;
        }
    }

    return -1;
}

int FileManager::selectBlock(int p)
{
    for(int i =0; i < pieces[p].blocks.size(); i ++){
        Block currBlock = pieces[p].blocks[i];
        if(currBlock.received != true || currBlock.requested != true){
            return i;
        }
    }

    return -1;
}

int FileManager::calcBlockLength(int p, int b)
{

    if(pieces.size()-1 == p && pieces[p].blocks.size()-1 == b){
        qDebug() << "FINAL PIECE";
        return totalSize % BLOCK_SIZE ;
    }else{
        return BLOCK_SIZE;
    }
}

void FileManager::requestNextBlock(Client *c, int pieceIndex)
{
    Piece &piece = pieces[pieceIndex];

    for(int i = 0; i < piece.blocks.size(); i++){
        Block &block = piece.blocks[i];
        if(block.requested || block.received)
            continue;

        //c->sendRequest(pieceIndex, i);
        block.requested = true;
        break;
    }
}

void FileManager::blockRequested(int p, int b)
{
    pieces[p].blocks[b].requested = true;

    //check if all blocks in piece were requested
    pieces[p].completed = true;
    for(int i = 0; i < pieces[p].blocks.size(); i++){
        if(pieces[p].blocks[i].requested == false){
            pieces[p].completed = false;
            break;
        }
    }
}

bool FileManager::blockRecieved(int p, int b)
{
    pieces[p].blocks[b].received = true;

    return true;
}

void FileManager::writeRequest(quint32 &index, quint32 &offset, QByteArray &data)
{
    WriteRequest request;
    request.pieceIndex = index;
    request.offset = offset;
    request.data = data;

    QMutexLocker Locker(&mutex);
    writeRequests << request;
}




bool FileManager::createFiles()
{
    qDebug() << "creating files...";
    //TODO multi file torrent
    if(true){
        //QDir

        QFile *file = new QFile(downloadDest + torrent->getFileName());
        if(!file->open(QFile::ReadWrite)){
            delete file;
            return false;
        }
        if(!file->resize(torrent->getLength())){
            return false;
        }

        fileList.append(file);
        file->close();

    }



    return true;
}

void FileManager::peerUnchoked()
{
    qDebug() << "Peer unchoked, beginning requests";
}



bool FileManager::writeBlock(quint32 &index, quint32 &offset, QByteArray &data)
{
    QFile *file = fileList.first();
    qDebug() << "data size: " << data.size();
    pieces[index].blocks[offset/BLOCK_SIZE].received = true;

    if(!file->open(QFile::ReadWrite)){
        return false;
    }

    if(index == pieces.size() && offset/BLOCK_SIZE == pieces[index].blocks.size()){
        //qDebug() << "writing final block";
    }
    quint32 startIndex = (pieceLength * index) + offset;
    qDebug() << "write start index: " << startIndex;
    //seek write index
    file->seek(startIndex);
    file->write(data);


    file->close();
    return true;
}

QByteArray FileManager::readBlock(quint32 index, quint32 offset, quint32 length)
{
    QFile *file = fileList.first();
    QByteArray data;

    quint64 startIndex = (index * pieceLength) + offset;
    qDebug() << "read start index: " << startIndex;
    if(!file->open(QFile::ReadWrite)){

    }

    file->seek(startIndex);
    data = file->read(length);

    file->close();
    return data;
}

bool FileManager::verifyPiece(quint32 index)
{
    QByteArray fullPiece = readBlock(index, 0, pieceLength);
    QByteArray pieceSum = QCryptographicHash::hash(fullPiece, QCryptographicHash::Sha1);

    if(pieceSum != pieces[index].hash){
        qDebug() << "INVALID PIECE HASH";
        resetInvalidPiece(index);
        return false;
    } else{
        qDebug() << "VALID PIECE HASH";
    }

    return true;

}


void FileManager::addVerifyPiece(quint32 index)
{
    QMutexLocker locker(&mutex);
    verifyRequests << index;
}

void FileManager::resetInvalidPiece(quint32 index)
{
    Piece p = pieces[index];

    p.completed = false;
    for(int i = 0; i < p.blocks.size(); i ++){
        Block b = p.blocks[i];
        b.received = false;
        b.requested = false;
    }
}

void FileManager::verifyList()
{

}

int FileManager::remainingBlocks(const int index) const
{
    int total = 0;
    for(int i = 0; i < pieces[index].blocks.size(); i++){
        Block currBlock = pieces[index].blocks[i];
        if(!currBlock.received)
            total++;
    }
    return total;
}
