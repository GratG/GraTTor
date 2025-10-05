#include "filemanager.h"

#include <QDebug>
FileManager::FileManager() {}

void FileManager::start()
{
    createFiles();
}

void FileManager::setTorrent(Torrent *t)
{
    torrent = t;
}

void FileManager::setPath(const QString &p)
{
    downloadDest = p;
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

bool FileManager::writeBlock(qint32 &index, qint32 &offset, QByteArray &data)
{
    QFile *file = fileList.first();

    if(!file->open(QFile::ReadWrite)){
        return false;
    }
    //figure out index/offset

    file->write(data);


    file->close();
    return true;
}
