#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QUrl>
#include "torrent.h"
#include "tracker.h"

class Client : public QObject {
    Q_OBJECT
    public:
        Client(QObject* parent = nullptr);

        bool setTorrent(const QString &fileName);
    private:
        Torrent *torrent;
        Tracker *tracker;



};

#endif // CLIENT_H
