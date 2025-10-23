#include "filemanager.h"

#include <QDebug>


const int BLOCK_SIZE = 16 * 1024; //16384

FileManager::FileManager() {}


void FileManager::start()
{
    createFiles();
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
        qDebug() << numBlocks;
        pieces[i].blocks.resize(numBlocks);
    }

}

void FileManager::setPath(const QString &p)
{
    downloadDest = p;
}

int FileManager::selectNextPiece(QBitArray &availablePieces)
{
    qDebug() << pieces.size();
    qDebug() << availablePieces.size();
    for (int i = 0; i < pieces.size(); i++){
        if(pieces[i].completed)
            continue;
        if(availablePieces.testBit(i)){
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

    pieces[index].blocks[offset/BLOCK_SIZE].received = true;

    if(!file->open(QFile::ReadWrite)){
        return false;
    }

    qint32 startIndex = (pieceLength * index) + offset;
    qDebug() << "start index: " << startIndex;
    //seek write index
    file->seek(startIndex);
    file->write(data);


    file->close();
    return true;
}
