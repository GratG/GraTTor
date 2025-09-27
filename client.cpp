#include "client.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>

Client::Client(QObject* parent) : QObject(parent){
    qDebug() << "client created...";
}

bool Client::setTorrent(const QString &fileName)
{
    torrent = new Torrent(fileName);
    torrent->extractBEncode();

    tracker = new Tracker(torrent);
    tracker->request();

    return true;
}

