#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QFile>
#include <QList>

#include "torrent.h"

class Torrent;
class FileManager {
    public:
        FileManager();
        void start();
        void setTorrent(Torrent *t);
        void setPath(const QString &p);

        bool writeBlock(qint32 &index, qint32 &offset, QByteArray &data);
    private:
        Torrent *torrent;
        QString downloadDest;
        QList<QFile*> fileList;


        bool createFiles();



};

#endif // FILEMANAGER_H
