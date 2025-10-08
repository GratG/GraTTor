#include "torrent.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
Torrent::Torrent(QString fileDir) {
    fileName = fileDir;

}



bool Torrent::extractBEncode()
{
    bencoding bencoder;
    bencoder.loadFile(fileName);
    // obtains bencoded data from file...
    QList<QVariant> bencodedData = bencoder.decode();

    //creates a SHA1 cryptographic hash from the original bencoded info data
    metaInfo.sha1 = QCryptographicHash::hash(bencoder.infoSection(), QCryptographicHash::Sha1);

    // splits data into 2 hash maps (main hash and the info hash)
    QHash mainHash = bencodedData[0].toHash();
    QHash infoHash = mainHash["info"].toHash();

    //assign first announce string to metainfo announce value
    metaInfo.announce = QString::fromUtf8(mainHash["announce"].toByteArray());

    announceList.append(mainHash["announce"].toString());

    //if there are more announce trackers (announce-list), append those to the announce list
    if(mainHash.contains("announce-list")){
        QList<QVariant> l =  mainHash["announce-list"].toList();
        for(int i=0; i< l.size(); i++){
            announceList.append(l[i].toList()[0].toString());
        }
    }
    comment = mainHash["comment"].toString();
    createdBy = mainHash["created by"].toString();

    //assign info hash data (mainly pieces and length)


    if(infoHash.contains("files")){
        //fileForm = MultiFileForm;
        //TODO implement multiple file support

    } else if(infoHash.contains("length")){
        //fileForm = SingleFileForm;
        metaInfo.info.length  = infoHash["length"].toInt();
        metaInfo.info.name = infoHash["name"].toString();
        metaInfo.info.pieceLength = infoHash["piece length"].toInt();
        qDebug() << "File length: " << metaInfo.info.length;
        qDebug() << "piece length: " << metaInfo.info.pieceLength;
        /* The pieces field is broken up into SHA1 hashes
         * each hash being 20bytes long.
         * Here we read the pieces string and assign it properly
        */

        metaInfo.info.pieces = infoHash["pieces"].toByteArray();
        QByteArray data = metaInfo.info.pieces;

        for(int i=0; i < data.size(); i+= 20){
            metaInfo.pieceHashes.append(data.mid(i, 20));
        }
        qDebug() << "Piece hashes: ";
        for(int i = 0; i< metaInfo.pieceHashes.size(); i++){
            qDebug() << metaInfo.pieceHashes[i].toHex();
        }

    }


    return true;

}

QString Torrent::getAnnounce(int index) const
{
    //returns a value from the announce list
    return announceList[index];
}

QByteArray Torrent::getPieces() const
{
    return "p";
}

QList<QByteArray> Torrent::getPieceHashes() const
{
    return metaInfo.pieceHashes;
}

QString Torrent::getAnnounce() const
{
    //returns the main announce value
    return metaInfo.announce;
}

QByteArray Torrent::getSHA1() const
{
    return metaInfo.sha1;
}

uint64_t Torrent::getLength() const
{
    return metaInfo.info.length;
}

QString Torrent::getFileName()
{
    //TODO multi file...
    return metaInfo.info.name;
}

qint64 Torrent::getPieceLength() const
{
    return metaInfo.info.pieceLength;
}
