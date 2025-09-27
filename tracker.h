#ifndef TRACKER_H
#define TRACKER_H

#include <QObject>
#include <QNetworkAccessManager>

#include "torrent.h"

class Torrent;

class Tracker : public QObject {
    Q_OBJECT
    public:
        Tracker(Torrent *t, QObject *parent = nullptr);
        void request();

    private:
        QString addr;
        Torrent *torrent;
        //QNetworkAccessManager httpRequest;

    private slots:
        void onReplyFinished();







};

#endif // TRACKER_H
